#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/TcpServer.h>

#include <boost/bind.hpp>

using namespace muduo;
using namespace muduo::net;

class EchoServer
{
 public:
  EchoServer(EventLoop* loop, const InetAddress& listenAddr)
    : loop_(loop),
      server_(loop, listenAddr)
  {
  }

  void start()
  {
    server_.start();
  }
  // void stop();

 private:
  EventLoop* loop_;
  TcpServer server_;
};

void threadFunc(uint16_t port)
{
}

int main()
{
  EventLoop loop;
  InetAddress listenAddr(2000);
  EchoServer server(&loop, listenAddr);
  server.start();

  loop.loop();
}

