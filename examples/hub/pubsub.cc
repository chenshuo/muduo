#include "pubsub.h"
#include "codec.h"

#include <boost/bind.hpp>

using namespace muduo;
using namespace muduo::net;
using namespace pubsub;

PubSubClient::PubSubClient(EventLoop* loop,
                           const InetAddress& hubAddr,
                           const string& name)
  : client_(loop, hubAddr, name)
{
  // FIXME: dtor is not thread safe
  client_.setConnectionCallback(
      boost::bind(&PubSubClient::onConnection, this, _1));
  client_.setMessageCallback(
      boost::bind(&PubSubClient::onMessage, this, _1, _2, _3));
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

bool PubSubClient::subscribe(const string& topic, const SubscribeCallback& cb)
{
  string message = "sub " + topic + "\r\n";
  subscribeCallback_ = cb;
  return send(message);
}

void PubSubClient::unsubscribe(const string& topic)
{
  string message = "unsub " + topic + "\r\n";
  send(message);
}


bool PubSubClient::publish(const string& topic, const string& content)
{
  string message = "pub " + topic + "\r\n" + content + "\r\n";
  return send(message);
}

void PubSubClient::onConnection(const TcpConnectionPtr& conn)
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

void PubSubClient::onMessage(const TcpConnectionPtr& conn,
                             Buffer* buf,
                             Timestamp receiveTime)
{
  ParseResult result = kSuccess;
  while (result == kSuccess)
  {
    string cmd;
    string topic;
    string content;
    result = parseMessage(buf, &cmd, &topic, &content);
    if (result == kSuccess)
    {
      if (cmd == "pub" && subscribeCallback_)
      {
        subscribeCallback_(topic, content, receiveTime);
      }
    }
    else if (result == kError)
    {
      conn->shutdown();
    }
  }
}

bool PubSubClient::send(const string& message)
{
  bool succeed = false;
  if (conn_ && conn_->connected())
  {
    conn_->send(message);
    succeed = true;
  }
  return succeed;
}
