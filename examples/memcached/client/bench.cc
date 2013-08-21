#include <muduo/base/CountDownLatch.h>
#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/EventLoopThreadPool.h>
#include <muduo/net/TcpClient.h>

#include <boost/bind.hpp>

#include <stdio.h>

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
         int keynum,
         int valuelen,
         CountDownLatch* connected,
         CountDownLatch* finished)
    : name_(name),
      client_(loop, serverAddr, name),
      op_(op),
      sent_(0),
      acked_(0),
      requests_(requests),
      keynum_(keynum),
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
      finished_->countDown();
    }
  }

  void fill(Buffer* buf)
  {
    char req[256];
    if (op_ == kSet)
    {
      snprintf(req, sizeof req, "set %s%d 42 0 %d\r\n", name_.c_str(), sent_ % keynum_, valuelen_);
      ++sent_;
      buf->append(req);
      buf->append(value_);
    }
    else
    {
      snprintf(req, sizeof req, "get %s%d\r\n", name_.c_str(), sent_ % keynum_);
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
  const int keynum_;
  const int valuelen_;
  string value_;
  CountDownLatch* const connected_;
  CountDownLatch* const finished_;
};

int main(int argc, char* argv[])
{
  Logger::setLogLevel(Logger::WARN);

  InetAddress serverAddr(argv[1], static_cast<uint16_t>(atoi(argv[2])));
  LOG_WARN << "Connecting " << serverAddr.toIpPort();

  EventLoop loop;
  EventLoopThreadPool pool(&loop);

  int clients = 100;
  int threads = 4;
  int requests = 100000;
  int keynum = 10000;
  int valuelen = 100;

  double memoryMiB = 1.0 * clients * keynum * (32+72+valuelen+8) / 1024 / 1024;
  LOG_WARN << "expected memcached-debug memory usage in MiB " << int(memoryMiB);

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
                                Client::kGet,
                                requests,
                                keynum,
                                valuelen,
                                &connected,
                                &finished));
  }
  connected.wait();
  LOG_WARN << "All connected";
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
