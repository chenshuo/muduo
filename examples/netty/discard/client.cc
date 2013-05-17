#include <muduo/net/TcpClient.h>

#include <muduo/base/Logging.h>
#include <muduo/base/Thread.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>

#include <boost/bind.hpp>

#include <utility>

#include <stdio.h>
#include <unistd.h>

using namespace muduo;
using namespace muduo::net;

class DiscardClient : boost::noncopyable
{
 public:
  DiscardClient(EventLoop* loop, const InetAddress& listenAddr, int size)
    : loop_(loop),
      client_(loop, listenAddr, "DiscardClient"),
      message_(size, 'H')
  {
    client_.setConnectionCallback(
        boost::bind(&DiscardClient::onConnection, this, _1));
    client_.setMessageCallback(
        boost::bind(&DiscardClient::onMessage, this, _1, _2, _3));
    client_.setWriteCompleteCallback(
        boost::bind(&DiscardClient::onWriteComplete, this, _1));
    //client_.enableRetry();
  }

  void connect()
  {
    client_.connect();
  }

 private:
  void onConnection(const TcpConnectionPtr& conn)
  {
    LOG_TRACE << conn->localAddress().toIpPort() << " -> "
        << conn->peerAddress().toIpPort() << " is "
        << (conn->connected() ? "UP" : "DOWN");

    if (conn->connected())
    {
      conn->setTcpNoDelay(true);
      conn->send(message_);
    }
    else
    {
      loop_->quit();
    }
  }

  void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time)
  {
    buf->retrieveAll();
  }

  void onWriteComplete(const TcpConnectionPtr& conn)
  {
    LOG_INFO << "write complete " << message_.size();
    conn->send(message_);
  }

  EventLoop* loop_;
  TcpClient client_;
  string message_;
};

int main(int argc, char* argv[])
{
  LOG_INFO << "pid = " << getpid() << ", tid = " << CurrentThread::tid();
  if (argc > 1)
  {
    EventLoop loop;
    InetAddress serverAddr(argv[1], 2009);

    int size = 256;
    if (argc > 2)
    {
      size = atoi(argv[2]);
    }

    DiscardClient client(&loop, serverAddr, size);
    client.connect();
    loop.loop();
  }
  else
  {
    printf("Usage: %s host_ip [msg_size]\n", argv[0]);
  }
}

