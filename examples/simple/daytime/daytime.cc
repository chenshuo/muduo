#include "examples/simple/daytime/daytime.h"

#include "muduo/base/Logging.h"
#include "muduo/net/EventLoop.h"

using namespace muduo;
using namespace muduo::net;

DaytimeServer::DaytimeServer(EventLoop* loop,
                             const InetAddress& listenAddr)
  : server_(loop, listenAddr, "DaytimeServer")
{
  server_.setConnectionCallback(
      std::bind(&DaytimeServer::onConnection, this, _1));
  server_.setMessageCallback(
      std::bind(&DaytimeServer::onMessage, this, _1, _2, _3));
}

void DaytimeServer::start()
{
  server_.start();
}

void DaytimeServer::onConnection(const TcpConnectionPtr& conn)
{
  LOG_INFO << "DaytimeServer - " << conn->peerAddress().toIpPort() << " -> "
           << conn->localAddress().toIpPort() << " is "
           << (conn->connected() ? "UP" : "DOWN");
  if (conn->connected())
  {
    conn->send(Timestamp::now().toFormattedString() + "\n");
    conn->shutdown();
  }
}

void DaytimeServer::onMessage(const TcpConnectionPtr& conn,
                              Buffer* buf,
                              Timestamp time)
{
  string msg(buf->retrieveAllAsString());
  LOG_INFO << conn->name() << " discards " << msg.size()
           << " bytes received at " << time.toString();
}

