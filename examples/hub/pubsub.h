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
  typedef boost::function<void (const string& content, Timestamp)> SubscribeCallback;
  typedef boost::function<void (PubSubClient*)> ConnectionCallback;

  PubSubClient(muduo::net::EventLoop* loop,
               const muduo::net::InetAddress& hubAddr,
               const string& name);
  void start();
  void stop();
  bool connected() const;

  void subscribe(const string& topic, const SubscribeCallback& cb);
  void unsubscribe(const string& topic);
  void publish(const string& topic, const string& content);

 private:
  muduo::net::EventLoop* loop_;
  muduo::net::TcpClient client_;
};
}

#endif  // MUDUO_EXAMPLES_HUB_PUBSUB_H
