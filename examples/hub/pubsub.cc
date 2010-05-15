#include "pubsub.h"

#include <boost/bind.hpp>

using namespace muduo;
using namespace muduo::net;
using namespace pubsub;

PubSubClient::PubSubClient(EventLoop* loop,
                           const InetAddress& hubAddr,
                           const string& name)
  : loop_(loop),
    client_(loop, hubAddr, name)
{
  // FIXME: dtor is not thread safe
  client_.setConnectionCallback(
      boost::bind(&PubSubClient::onConnection, this, _1));
}

void PubSubClient::start()
{
  client_.connect();
}

void PubSubClient::stop()
{
  client_.disconnect();
}

bool PubSubClient::connected() const
{
  return conn_ && conn_->connected();
}

bool PubSubClient::publish(const string& topic, const string& content)
{
  string message = "pub " + topic + "\r\n" + content + "\r\n";
  bool succeed = false;
  if (conn_ && conn_->connected())
  {
    conn_->send(message);
    succeed = true;
  }
  return succeed;
}

void PubSubClient::onConnection(const muduo::net::TcpConnectionPtr& conn)
{
  if (conn->connected())
  {
    conn_ = conn;
    // FIXME: re-sub
  }
  else
  {
    conn_.reset();
  }
  if (connectionCallback_)
  {
    connectionCallback_(this);
  }
}

