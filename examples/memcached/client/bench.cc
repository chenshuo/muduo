#include <muduo/base/CountDownLatch.h>
#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/EventLoopThreadPool.h>
#include <muduo/net/TcpClient.h>

#include <boost/bind.hpp>
#include <boost/program_options.hpp>
#include <iostream>

#include <stdio.h>

namespace po = boost::program_options;
using namespace muduo;
using namespace muduo::net;

class Client : boost::noncopyable
{
 public:
  enum Operation
  {
    kGet,
    kSet,
  };

  Client(const string& name,
         EventLoop* loop,
         const InetAddress& serverAddr,
         Operation op,
         int requests,
         int keys,
         int valuelen,
         CountDownLatch* connected,
         CountDownLatch* finished)
    : name_(name),
      client_(loop, serverAddr, name),
      op_(op),
      sent_(0),
      acked_(0),
      requests_(requests),
      keys_(keys),
      valuelen_(valuelen),
      value_(valuelen_, 'a'),
      connected_(connected),
      finished_(finished)
  {
    value_ += "\r\n";
    client_.setConnectionCallback(boost::bind(&Client::onConnection, this, _1));
    client_.setMessageCallback(boost::bind(&Client::onMessage, this, _1, _2, _3));
    client_.connect();
  }

  void send()
  {
    Buffer buf;
    fill(&buf);
    conn_->send(&buf);
  }

 private:

  void onConnection(const TcpConnectionPtr& conn)
  {
    if (conn->connected())
    {
      conn_ = conn;
      connected_->countDown();
    }
    else
    {
      conn_.reset();
      client_.getLoop()->queueInLoop(boost::bind(&CountDownLatch::countDown, finished_));
    }
  }

  void onMessage(const TcpConnectionPtr& conn,
                 Buffer* buffer,
                 Timestamp receiveTime)
  {
    if (op_ == kSet)
    {
      while (buffer->readableBytes() > 0)
      {
        const char* crlf = buffer->findCRLF();
        if (crlf)
        {
          buffer->retrieveUntil(crlf+2);
          ++acked_;
          if (sent_ < requests_)
          {
            send();
          }
        }
        else
        {
          break;
        }
      }
    }
    else
    {
      while (buffer->readableBytes() > 0)
      {
        const char* end = static_cast<const char*>(memmem(buffer->peek(),
                                                          buffer->readableBytes(),
                                                          "END\r\n", 5));
        if (end)
        {
          buffer->retrieveUntil(end+5);
          ++acked_;
          if (sent_ < requests_)
          {
            send();
          }
        }
        else
        {
          break;
        }
      }
    }
    if (acked_ == requests_)
    {
      conn_->shutdown();
    }
  }

  void fill(Buffer* buf)
  {
    char req[256];
    if (op_ == kSet)
    {
      snprintf(req, sizeof req, "set %s%d 42 0 %d\r\n", name_.c_str(), sent_ % keys_, valuelen_);
      ++sent_;
      buf->append(req);
      buf->append(value_);
    }
    else
    {
      snprintf(req, sizeof req, "get %s%d\r\n", name_.c_str(), sent_ % keys_);
      ++sent_;
      buf->append(req);
    }
  }

  string name_;
  TcpClient client_;
  TcpConnectionPtr conn_;
  const Operation op_;
  int sent_;
  int acked_;
  const int requests_;
  const int keys_;
  const int valuelen_;
  string value_;
  CountDownLatch* const connected_;
  CountDownLatch* const finished_;
};

int main(int argc, char* argv[])
{
  Logger::setLogLevel(Logger::WARN);

  uint16_t tcpport = 11211;
  string hostIp = "127.0.0.1";
  int threads = 4;
  int clients = 100;
  int requests = 100000;
  int keys = 10000;
  bool set = false;

  po::options_description desc("Allowed options");
  desc.add_options()
      ("help,h", "Help")
      ("port,p", po::value<uint16_t>(&tcpport), "TCP port")
      ("ip,i", po::value<string>(&hostIp), "Host IP")
      ("threads,t", po::value<int>(&threads), "Number of worker threads")
      ("clients,c", po::value<int>(&clients), "Number of concurrent clients")
      ("requests,r", po::value<int>(&requests), "Number of requests per clients")
      ("keys,k", po::value<int>(&keys), "Number of keys per clients")
      ("set,s", "Get or Set")
      ;

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help"))
  {
    std::cout << desc << "\n";
    return 0;
  }
  set = vm.count("set");

  InetAddress serverAddr(hostIp, tcpport);
  LOG_WARN << "Connecting " << serverAddr.toIpPort();

  EventLoop loop;
  EventLoopThreadPool pool(&loop, "bench-memcache");

  int valuelen = 100;
  Client::Operation op = set ? Client::kSet : Client::kGet;

  double memoryMiB = 1.0 * clients * keys * (32+80+valuelen+8) / 1024 / 1024;
  LOG_WARN << "estimated memcached-debug memory usage " << int(memoryMiB) << " MiB";

  pool.setThreadNum(threads);
  pool.start();

  char buf[32];
  CountDownLatch connected(clients);
  CountDownLatch finished(clients);
  boost::ptr_vector<Client> holder;
  for (int i = 0; i < clients; ++i)
  {
    snprintf(buf, sizeof buf, "%d-", i+1);
    holder.push_back(new Client(buf,
                                pool.getNextLoop(),
                                serverAddr,
                                op,
                                requests,
                                keys,
                                valuelen,
                                &connected,
                                &finished));
  }
  connected.wait();
  LOG_WARN << clients << " clients all connected";
  Timestamp start = Timestamp::now();
  for (int i = 0; i < clients; ++i)
  {
    holder[i].send();
  }
  finished.wait();
  Timestamp end = Timestamp::now();
  LOG_WARN << "All finished";
  double seconds = timeDifference(end, start);
  LOG_WARN << seconds << " sec";
  LOG_WARN << 1.0 * clients * requests / seconds << " QPS";
}
