#include "examples/simple/time/time.h"

#include "muduo/base/Logging.h"
#include "muduo/net/Endian.h"

using namespace muduo;
using namespace muduo::net;

TimeServer::TimeServer(muduo::net::EventLoop* loop,
                             const muduo::net::InetAddress& listenAddr)
  : server_(loop, listenAddr, "TimeServer")
{
  server_.setConnectionCallback(
      std::bind(&TimeServer::onConnection, this, _1));
  server_.setMessageCallback(
      std::bind(&TimeServer::onMessage, this, _1, _2, _3));
}

void TimeServer::start()
{
  server_.start();
}

void TimeServer::onConnection(const muduo::net::TcpConnectionPtr& conn)
{
  LOG_INFO << "TimeServer - " << conn->peerAddress().toIpPort() << " -> "
           << conn->localAddress().toIpPort() << " is "
           << (conn->connected() ? "UP" : "DOWN");
  if (conn->connected())
  {
    time_t now = ::time(NULL);
    int32_t be32 = sockets::hostToNetwork32(static_cast<int32_t>(now));
    conn->send(&be32, sizeof be32);
    conn->shutdown();
  }
}

void TimeServer::onMessage(const muduo::net::TcpConnectionPtr& conn,
                 muduo::net::Buffer* buf,
                 muduo::Timestamp time)
{
  string msg(buf->retrieveAllAsString());
  LOG_INFO << conn->name() << " discards " << msg.size()
           << " bytes received at " << time.toString();
}

