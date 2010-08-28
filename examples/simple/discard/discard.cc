#include "discard.h"

#include <muduo/base/Logging.h>

#include <boost/bind.hpp>

using namespace muduo;
using namespace muduo::net;

DiscardServer::DiscardServer(muduo::net::EventLoop* loop,
                             const muduo::net::InetAddress& listenAddr)
  : loop_(loop),
    server_(loop, listenAddr, "DiscardServer")
{
  server_.setConnectionCallback(
      boost::bind(&DiscardServer::onConnection, this, _1));
  server_.setMessageCallback(
      boost::bind(&DiscardServer::onMessage, this, _1, _2, _3));
}

void DiscardServer::start()
{
  server_.start();
}

void DiscardServer::onConnection(const muduo::net::TcpConnectionPtr& conn)
{
  LOG_INFO << "DiscardServer - " << conn->peerAddress().toHostPort() << " -> "
    << conn->localAddress().toHostPort() << " is "
    << (conn->connected() ? "UP" : "DOWN");
}

void DiscardServer::onMessage(const muduo::net::TcpConnectionPtr& conn,
                 muduo::net::Buffer* buf,
                 muduo::Timestamp time)
{
  string msg(buf->retrieveAsString());
  LOG_INFO << conn->name() << " discards " << msg.size() << " bytes at " << time.toString();
}

