#include "chargen.h"

#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>

#include <boost/bind.hpp>
#include <stdio.h>

using namespace muduo;
using namespace muduo::net;

ChargenServer::ChargenServer(EventLoop* loop,
                             const InetAddress& listenAddr,
                             bool print)
  : server_(loop, listenAddr, "ChargenServer"),
    transferred_(0),
    startTime_(Timestamp::now())
{
  server_.setConnectionCallback(
      boost::bind(&ChargenServer::onConnection, this, _1));
  server_.setMessageCallback(
      boost::bind(&ChargenServer::onMessage, this, _1, _2, _3));
  server_.setWriteCompleteCallback(
      boost::bind(&ChargenServer::onWriteComplete, this, _1));
  if (print)
  {
    loop->runEvery(3.0, boost::bind(&ChargenServer::printThroughput, this));
  }

  string line;
  for (int i = 33; i < 127; ++i)
  {
    line.push_back(char(i));
  }
  line += line;

  for (size_t i = 0; i < 127-33; ++i)
  {
    message_ += line.substr(i, 72) + '\n';
  }
}

void ChargenServer::start()
{
  server_.start();
}

void ChargenServer::onConnection(const TcpConnectionPtr& conn)
{
  LOG_INFO << "ChargenServer - " << conn->peerAddress().toIpPort() << " -> "
           << conn->localAddress().toIpPort() << " is "
           << (conn->connected() ? "UP" : "DOWN");
  if (conn->connected())
  {
    conn->setTcpNoDelay(true);
    conn->send(message_);
  }
}

void ChargenServer::onMessage(const TcpConnectionPtr& conn,
                              Buffer* buf,
                              Timestamp time)
{
  string msg(buf->retrieveAllAsString());
  LOG_INFO << conn->name() << " discards " << msg.size()
           << " bytes received at " << time.toString();
}

void ChargenServer::onWriteComplete(const TcpConnectionPtr& conn)
{
  transferred_ += message_.size();
  conn->send(message_);
}

void ChargenServer::printThroughput()
{
  Timestamp endTime = Timestamp::now();
  double time = timeDifference(endTime, startTime_);
  printf("%4.3f MiB/s\n", static_cast<double>(transferred_)/time/1024/1024);
  transferred_ = 0;
  startTime_ = endTime;
}

