#include "echo.h"

#include <muduo/base/Logging.h>

#include <boost/bind.hpp>

using namespace muduo;
using namespace muduo::net;

const int kMaxConnections = 1024*1024;

EchoServer::EchoServer(EventLoop* loop,
                       const InetAddress& listenAddr)
  : loop_(loop),
    server_(loop, listenAddr, "EchoServer")
{
  server_.setConnectionCallback(
      boost::bind(&EchoServer::onConnection, this, _1));
  server_.setMessageCallback(
      boost::bind(&EchoServer::onMessage, this, _1, _2, _3));
}

void EchoServer::start()
{
  server_.start();
}

int numConnected = 0;
void EchoServer::onConnection(const TcpConnectionPtr& conn)
{
  LOG_INFO << "EchoServer - " << conn->peerAddress().toHostPort() << " -> "
    << conn->localAddress().toHostPort() << " is "
    << (conn->connected() ? "UP" : "DOWN");
  if (conn->connected())
  {
    ++numConnected;
    if (numConnected > kMaxConnections)
    {
      conn->shutdown();
    }
  }
  else
  {
    --numConnected;
  }
  LOG_INFO << "numConnected = " << numConnected;
}

void EchoServer::onMessage(const TcpConnectionPtr& conn,
                           Buffer* buf,
                           Timestamp time)
{
  string msg(buf->retrieveAsString());
  LOG_INFO << conn->name() << " echo " << msg.size() << " bytes at " << time.toString();
  conn->send(msg);
}

