#ifndef MUDUO_EXAMPLES_HUB_CODEC_H
#define MUDUO_EXAMPLES_HUB_CODEC_H

// internal header file

#include <muduo/base/Types.h>
#include <muduo/net/Callbacks.h>

#include <boost/noncopyable.hpp>

namespace pubsub
{
using muduo::string;

class PubSubCodec : boost::noncopyable
{
 public:
  typedef boost::function<void (const string& topic)> SubscribeCallback;
  typedef SubscribeCallback UnsubscribeCallback;
  typedef boost::function<void (const string& topic,
                                const string& content)> PublishCallback;

  void onMessage(const muduo::net::TcpConnectionPtr& conn,
                 muduo::net::Buffer* buf,
                 muduo::Timestamp receiveTime);

  // LoginCallback loginCallback_;
  PublishCallback publishCallback_;
  SubscribeCallback subscribeCallback_;
  UnsubscribeCallback unsubscribeCallback_;
};

}

#endif  // MUDUO_EXAMPLES_HUB_CODEC_H

