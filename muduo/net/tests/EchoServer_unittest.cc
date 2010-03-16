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
        boost::bind(&EchoServer::onMessage, this, _1, _2));
    server_.setThreadNum(0);
  }

  void start()
  {
    server_.start();
  }
  // void stop();

 private:
  TcpConnectionPtr first;
  TcpConnectionPtr second;
  void onConnection(const TcpConnectionPtr& conn)
  {
    printf("conn %s -> %s %s\n",
        conn->peerAddress().toHostPort().c_str(),
        conn->localAddress().toHostPort().c_str(),
        conn->connected() ? "UP" : "DOWN");
    if (!first)
      first = conn;
    else if (!second)
      second = conn;
  }

  void onMessage(const TcpConnectionPtr& conn, ChannelBuffer* buf)
  {
    string msg(buf->retrieveAsString());
    printf("recv %zu bytes '%s'", msg.size(), msg.c_str());
    // conn->send(buf);
    // conn->shutdown();
    // loop_->quit();
    if (second && conn == first)
    {
      second->shutdown();
      second.reset();
      first.reset();
      loop_->quit();
    }
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

