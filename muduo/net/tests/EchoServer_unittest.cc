#include <muduo/net/TcpServer.h>

#include <muduo/base/Thread.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>

#include <boost/bind.hpp>

#include <stdio.h>

using namespace muduo;
using namespace muduo::net;

class EchoServer
{
 public:
  EchoServer(EventLoop* loop, const InetAddress& listenAddr)
    : loop_(loop),
      server_(loop, listenAddr)
  {
    server_.setConnectionCallback(
        boost::bind(&EchoServer::onConnection, this, _1));
    server_.setMessageCallback(
        boost::bind(&EchoServer::onMessage, this, _1, _2, _3));
  }

  void start()
  {
    server_.start();
  }
  // void stop();

 private:
  void onConnection(TcpConnection*)
  {
  }

  void onMessage(TcpConnection*, const void* buf, ssize_t len)
  {
  }

  EventLoop* loop_;
  TcpServer server_;
};

void threadFunc(uint16_t port)
{
}

int main()
{
  printf("main(): pid = %d, tid = %d\n", getpid(), CurrentThread::tid());
  EventLoop loop;
  InetAddress listenAddr(2000);
  EchoServer server(&loop, listenAddr);

  server.start();

  loop.loop();
}

