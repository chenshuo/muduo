#ifndef MUDUO_EXAMPLES_HUB_PUBSUB_H
#define MUDUO_EXAMPLES_HUB_PUBSUB_H

#include <muduo/net/TcpClient.h>

namespace pubsub
{
using muduo::string;
using muduo::Timestamp;

class PubSubClient : boost::noncopyable
{
 public:
  typedef boost::function<void (const string& content, Timestamp)> Callback;
  PubSubClient(muduo::net::EventLoop* loop,
               const muduo::net::InetAddress& hubAddr);
  void subscribe(const string& topic, const Callback& cb);
  void unsubscribe(const string& topic);

 private:
  muduo::net::EventLoop* loop_;
  muduo::net::TcpClient client_;
};
}

#endif  // MUDUO_EXAMPLES_HUB_PUBSUB_H
