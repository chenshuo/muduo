#include "sudoku.h"

#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpClient.h>

#include <boost/bind.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

#include <fstream>

#include <stdio.h>

using namespace muduo;
using namespace muduo::net;

bool verify(const string& result)
{
  return true;
}

void runLocal(std::istream& in)
{
  Timestamp start(Timestamp::now());
  std::string line;
  int count = 0;
  int succeed = 0;
  while (getline(in, line))
  {
    if (line.size() == implicit_cast<size_t>(kCells))
    {
      ++count;
      if (verify(solveSudoku(line)))
      {
        ++succeed;
      }
    }
  }
  double elapsed = timeDifference(Timestamp::now(), start);
  printf("%.3f sec, %.3f us per sudoku.\n", elapsed, 1000 * 1000 * elapsed / count);
}

typedef std::vector<string> Input;
typedef boost::shared_ptr<Input> InputPtr;

InputPtr readInput(std::istream& in)
{
  InputPtr input(new Input);
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

typedef boost::function<void(const string&, double, int)> DoneCallback;

class SudokuClient : boost::noncopyable
{
 public:
  SudokuClient(EventLoop* loop,
               const InetAddress& serverAddr,
               const InputPtr& input,
               const string& name,
               const DoneCallback& cb
               )
    : name_(name),
      client_(loop, serverAddr, name_),
      input_(input),
      cb_(cb),
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

 private:
  void onConnection(const TcpConnectionPtr& conn)
  {
    if (conn->connected())
    {
      LOG_INFO << name_ << " connected";
      start_ = Timestamp::now();
      for (size_t i = 0; i < input_->size(); ++i)
      {
        LogStream buf;
        buf << i+1 << ":" << (*input_)[i] << "\r\n";
        conn->send(buf.buffer().data(), buf.buffer().length());
      }
      LOG_INFO << name_ << " sent requests";
    }
    else
    {
      LOG_INFO << name_ << " disconnected";
    }
  }

  void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp)
  {
    //LOG_DEBUG << buf->retrieveAllAsString();

    size_t len = buf->readableBytes();
    while (len >= kCells + 2)
    {
      const char* crlf = buf->findCRLF();
      if (crlf)
      {
        string response(buf->peek(), crlf);
        buf->retrieveUntil(crlf + 2);
        len = buf->readableBytes();
        ++count_;
        if (!verify(response))
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

    if (count_ == static_cast<int>(input_->size()))
    {
      LOG_INFO << name_ << " done.";
      double elapsed = timeDifference(Timestamp::now(), start_);
      cb_(name_, elapsed, count_);
      conn->shutdown();
    }
  }

  string name_;
  TcpClient client_;
  InputPtr input_;
  DoneCallback cb_;
  int count_;
  Timestamp start_;
};

Timestamp g_start;
int g_connections;
int g_finished;
EventLoop* g_loop;

void done(const string& name, double elapsed, int count)
{
  LOG_INFO << name << " " << elapsed << " seconds "
           << Fmt("%.3f", 1000 * 1000 * elapsed / count)
           << " us per request.";
  ++g_finished;
  if (g_finished == g_connections)
  {
    g_loop->runAfter(1.0, boost::bind(&EventLoop::quit, g_loop));
    double total = timeDifference(Timestamp::now(), g_start);
    LOG_INFO << "total " << total << " seconds, "
             << (total/g_connections) << " seconds per client";
  }
}

void runClient(std::istream& in, const InetAddress& serverAddr, int conn)
{
  InputPtr input(readInput(in));
  EventLoop loop;
  g_loop = &loop;
  g_connections = conn;

  g_start = Timestamp::now();
  boost::ptr_vector<SudokuClient> clients;
  for (int i = 0; i < conn; ++i)
  {
    Fmt f("client-%03d", i+1);
    string name(f.data(), f.length());
    clients.push_back(new SudokuClient(&loop, serverAddr, input, name, done));
    clients.back().connect();
  }

  loop.loop();
}

int main(int argc, char* argv[])
{
  int conn = 1;
  InetAddress serverAddr("127.0.0.1", 9981);
  const char* input = NULL;
  bool local = true;
  switch (argc)
  {
    case 4:
      conn = atoi(argv[3]);
      // FALL THROUGH
    case 3:
      serverAddr = InetAddress(argv[2], 9981);
      // FALL THROUGH
      local = false;
    case 2:
      input = argv[1];
      break;
    default:
      printf("Usage: %s input server_ip [connections]\n", argv[0]);
      return 0;
  }

  std::ifstream in(input);
  if (in)
  {
    if (local)
    {
      runLocal(in);
    }
    else
    {
      runClient(in, serverAddr, conn);
    }
  }
  else
  {
    printf("Cannot open %s\n", input);
  }
}
