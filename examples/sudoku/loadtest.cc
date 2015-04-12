#include "sudoku.h"

#include <muduo/base/Logging.h>
#include <muduo/base/FileUtil.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpClient.h>

#include <boost/bind.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/unordered_map.hpp>

#include <fstream>
#include <numeric>

#include "percentile.h"

#include <stdio.h>

using namespace muduo;
using namespace muduo::net;

typedef std::vector<string> Input;
typedef boost::shared_ptr<const Input> InputPtr;

InputPtr readInput(std::istream& in)
{
  boost::shared_ptr<Input> input(new Input);
  std::string line;
  while (getline(in, line))
  {
    if (line.size() == implicit_cast<size_t>(kCells))
    {
      input->push_back(line.c_str());
    }
  }
  return input;
}

class SudokuClient : boost::noncopyable
{
 public:
  SudokuClient(EventLoop* loop,
               const InetAddress& serverAddr,
               const InputPtr& input,
               const string& name,
               bool nodelay)
    : name_(name),
      tcpNoDelay_(nodelay),
      client_(loop, serverAddr, name_),
      input_(input),
      count_(0)
  {
    client_.setConnectionCallback(
        boost::bind(&SudokuClient::onConnection, this, _1));
    client_.setMessageCallback(
        boost::bind(&SudokuClient::onMessage, this, _1, _2, _3));
  }

  void connect()
  {
    client_.connect();
  }

  void send(int n)
  {
    assert(n > 0);
    if (!conn_)
      return;

    Timestamp now(Timestamp::now());
    for (int i = 0; i < n; ++i)
    {
      char buf[256];
      const string& req = (*input_)[count_ % input_->size()];
      int len = snprintf(buf, sizeof buf, "%s-%08d:%s\r\n",
                         name_.c_str(), count_, req.c_str());
      requests_.append(buf, len);
      sendTime_[count_] = now;
      ++count_;
    }

    conn_->send(&requests_);
  }

  void report(std::vector<int>* latency, int* infly)
  {
    latency->insert(latency->end(), latencies_.begin(), latencies_.end());
    latencies_.clear();
    *infly += static_cast<int>(sendTime_.size());
  }

 private:
  void onConnection(const TcpConnectionPtr& conn)
  {
    if (conn->connected())
    {
      LOG_INFO << name_ << " connected";
      if (tcpNoDelay_)
        conn->setTcpNoDelay(true);
      conn_ = conn;
    }
    else
    {
      LOG_INFO << name_ << " disconnected";
      conn_.reset();
      // FIXME: exit
    }
  }

  void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp recvTime)
  {
    size_t len = buf->readableBytes();
    while (len >= kCells + 2)
    {
      const char* crlf = buf->findCRLF();
      if (crlf)
      {
        string response(buf->peek(), crlf);
        buf->retrieveUntil(crlf + 2);
        len = buf->readableBytes();
        if (!verify(response, recvTime))
        {
          LOG_ERROR << "Bad response:" << response;
          conn->shutdown();
          break;
        }
      }
      else if (len > 100) // id + ":" + kCells + "\r\n"
      {
        LOG_ERROR << "Line is too long!";
        conn->shutdown();
        break;
      }
      else
      {
        break;
      }
    }
  }

  bool verify(const string& response, Timestamp recvTime)
  {
    size_t colon = response.find(':');
    if (colon != string::npos)
    {
      size_t dash = response.find('-');
      if (dash != string::npos && dash < colon)
      {
        int id = atoi(response.c_str()+dash+1);
        boost::unordered_map<int, Timestamp>::iterator sendTime = sendTime_.find(id);
        if (sendTime != sendTime_.end())
        {
          int64_t latency_us = recvTime.microSecondsSinceEpoch() - sendTime->second.microSecondsSinceEpoch();
          latencies_.push_back(static_cast<int>(latency_us));
          sendTime_.erase(sendTime);
        }
        else
        {
          LOG_ERROR << "Unknown id " << id << " of " << name_;
        }
      }
    }
    // FIXME
    return true;
  }

  const string name_;
  const bool tcpNoDelay_;
  TcpClient client_;
  TcpConnectionPtr conn_;
  Buffer requests_;
  const InputPtr input_;
  int count_;
  boost::unordered_map<int, Timestamp> sendTime_;
  std::vector<int> latencies_;
};

class SudokuLoadtest : boost::noncopyable
{
 public:
  SudokuLoadtest()
    : count_(0),
      ticks_(0),
      sofar_(0)
  {
  }

  void runClient(const InputPtr& input, const InetAddress& serverAddr, int rps, int conn, bool nodelay)
  {
    EventLoop loop;

    for (int i = 0; i < conn; ++i)
    {
      Fmt f("c%04d", i+1);
      string name(f.data(), f.length());
      clients_.push_back(new SudokuClient(&loop, serverAddr, input, name, nodelay));
      clients_.back().connect();
    }

    loop.runEvery(1.0 / kHz, boost::bind(&SudokuLoadtest::tick, this, rps));
    loop.runEvery(1.0, boost::bind(&SudokuLoadtest::tock, this));
    loop.loop();
  }

 private:
  void tick(int rps)
  {
    ++ticks_;
    int64_t reqs = rps * ticks_ / kHz - sofar_;
    sofar_ += reqs;

    if (reqs > 0)
    {
      for (boost::ptr_vector<SudokuClient>::iterator it = clients_.begin();
           it != clients_.end(); ++it)
      {
        it->send(static_cast<int>(reqs));
      }
    }
  }

  void tock()
  {
    std::vector<int> latencies;
    int infly = 0;
    for (boost::ptr_vector<SudokuClient>::iterator it = clients_.begin();
         it != clients_.end(); ++it)
    {
      it->report(&latencies, &infly);
    }

    Percentile p(latencies, infly);
    LOG_INFO << p.report();
    char buf[64];
    snprintf(buf, sizeof buf, "r%04d", count_);
    p.save(latencies, buf);
    ++count_;
  }

  boost::ptr_vector<SudokuClient> clients_;
  int count_;
  int64_t ticks_;
  int64_t sofar_;
  static const int kHz = 100;
};

int main(int argc, char* argv[])
{
  int conn = 1;
  int rps = 100;
  bool nodelay = false;
  InetAddress serverAddr("127.0.0.1", 9981);
  switch (argc)
  {
    case 6:
      nodelay = string(argv[5]) == "-n";
      // FALL THROUGH
    case 5:
      conn = atoi(argv[4]);
      // FALL THROUGH
    case 4:
      rps = atoi(argv[3]);
      // FALL THROUGH
    case 3:
      serverAddr = InetAddress(argv[2], 9981);
      // FALL THROUGH
    case 2:
      break;
    default:
      printf("Usage: %s input server_ip [requests_per_second] [connections] [-n]\n", argv[0]);
      return 0;
  }

  std::ifstream in(argv[1]);
  if (in)
  {
    InputPtr input(readInput(in));
    printf("%zd requests from %s\n", input->size(), argv[1]);
    SudokuLoadtest test;
    test.runClient(input, serverAddr, rps, conn, nodelay);
  }
  else
  {
    printf("Cannot open %s\n", argv[1]);
  }
}
