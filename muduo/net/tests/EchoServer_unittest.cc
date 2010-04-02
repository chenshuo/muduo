#include <muduo/net/TcpServer.h>

#include <muduo/base/Logging.h>
#include <muduo/base/Thread.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>

#include <boost/bind.hpp>

#include <mcheck.h>
#include <stdio.h>
#include <unistd.h>

using namespace muduo;
using namespace muduo::net;

int numThreads = 0;

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
    server_.setThreadNum(numThreads);
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
    LOG_TRACE << conn->peerAddress().toHostPort() << " -> "
        << conn->localAddress().toHostPort() << " is "
        << (conn->connected() ? "UP" : "DOWN");
    if (conn->connected())
    {
    if (!first)
      first = conn;
    else if (!second)
      second = conn;
    }
    else
    {
      if (first == conn)
        first.reset();
      else if (second == conn)
        second.reset();
    }
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
      // first->shutdown();
      first.reset();
      // second->shutdown();
      second.reset();
      loop_->quit();
    }
    if (conn != first && first)
      first->send(msg);
    if (conn != second && second)
      second->send(msg);
    // conn->send(buf);
    // conn->shutdown();
    // loop_->quit();
    /*
    if (conn == second)
    {
      sleep(10);
    }
    if (second && conn == first)
    {
      second->shutdown();
      second.reset();
      first.reset();
      // loop_->quit();
    }
    */
  }

  EventLoop* loop_;
  TcpServer server_;
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
  InetAddress listenAddr(2000);
  EchoServer server(&loop, listenAddr);

  server.start();

  loop.loop();
}

