#include <muduo/net/TcpClient.h>

#include <muduo/base/Logging.h>
#include <muduo/base/Thread.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>

#include <boost/bind.hpp>

#include <utility>

#include <mcheck.h>
#include <stdio.h>
#include <unistd.h>

using namespace muduo;
using namespace muduo::net;

int numThreads = 0;

class EchoClient
{
 public:
  EchoClient(EventLoop* loop, const InetAddress& listenAddr)
    : loop_(loop),
      client_(loop, listenAddr)
  {
    client_.setConnectionCallback(
        boost::bind(&EchoClient::onConnection, this, _1));
    client_.setMessageCallback(
        boost::bind(&EchoClient::onMessage, this, _1, _2, _3));
  }

  void connect()
  {
    client_.connect();
  }
  // void stop();

 private:
  void onConnection(const TcpConnectionPtr& conn)
  {
    LOG_TRACE << conn->peerAddress().toHostPort() << " -> "
        << conn->localAddress().toHostPort() << " is "
        << (conn->connected() ? "UP" : "DOWN");

    conn->send("hello\n");
  }

  void onMessage(const TcpConnectionPtr& conn, ChannelBuffer* buf, Timestamp time)
  {
    string msg(buf->retrieveAsString());
    LOG_TRACE << conn->name() << " recv " << msg.size() << " bytes";
    if (msg == "exit\n")
    {
      conn->send("bye\n");
      conn->shutdown();
    }
    if (msg == "quit\n")
    {
      loop_->quit();
    }
    sleep(2);
    conn->send(msg);
  }

  EventLoop* loop_;
  TcpClient client_;
};

void threadFunc(uint16_t port)
{
}

int main(int argc, char* argv[])
{
  mtrace();
  LOG_INFO << "pid = " << getpid() << ", tid = " << CurrentThread::tid();
  LOG_INFO << "sizeof TcpConnection = " << sizeof(TcpConnection);
  numThreads = argc - 1;
  EventLoop loop;
  InetAddress serverAddr("127.0.0.1", 2000);
  EchoClient client(&loop, serverAddr);

  client.connect();

  loop.loop();
}

