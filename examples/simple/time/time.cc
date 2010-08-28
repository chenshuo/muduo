#include "time.h"

#include <muduo/base/Logging.h>
#include <muduo/net/SocketsOps.h>

#include <boost/bind.hpp>

using namespace muduo;
using namespace muduo::net;

TimeServer::TimeServer(muduo::net::EventLoop* loop,
                             const muduo::net::InetAddress& listenAddr)
  : loop_(loop),
    server_(loop, listenAddr, "TimeServer")
{
  server_.setConnectionCallback(
      boost::bind(&TimeServer::onConnection, this, _1));
  server_.setMessageCallback(
      boost::bind(&TimeServer::onMessage, this, _1, _2, _3));
}

void TimeServer::start()
{
  server_.start();
}

void TimeServer::onConnection(const muduo::net::TcpConnectionPtr& conn)
{
  LOG_INFO << "TimeServer - " << conn->peerAddress().toHostPort() << " -> "
    << conn->localAddress().toHostPort() << " is "
    << (conn->connected() ? "UP" : "DOWN");
  int32_t now = sockets::hostToNetwork32(static_cast<int>(::time(NULL)));
  conn->send(&now, sizeof now);
  conn->shutdown();
}

void TimeServer::onMessage(const muduo::net::TcpConnectionPtr& conn,
                 muduo::net::Buffer* buf,
                 muduo::Timestamp time)
{
  string msg(buf->retrieveAsString());
  LOG_INFO << conn->name() << " discards " << msg.size() << " bytes at " << time.toString();
}

