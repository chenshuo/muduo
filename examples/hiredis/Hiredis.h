#ifndef MUDUO_EXAMPLES_HIREDIS_HIREDIS_H
#define MUDUO_EXAMPLES_HIREDIS_HIREDIS_H

#include <muduo/base/StringPiece.h>
#include <muduo/base/Types.h>
#include <muduo/net/Callbacks.h>
#include <muduo/net/InetAddress.h>

#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

#include <hiredis/async.h>
#include <hiredis/hiredis.h>

namespace muduo
{
namespace net
{
class Channel;
class EventLoop;
}
}

namespace hiredis
{

class Hiredis : boost::noncopyable,
                public::boost::enable_shared_from_this<Hiredis>
{
 public:
  typedef boost::function<void(const redisAsyncContext*, int)> ConnectCallback;
  typedef boost::function<void(const redisAsyncContext*, int)> DisconnectCallback;
  typedef boost::function<void(redisAsyncContext*, redisReply*, void*)> CommandCallback;

  Hiredis(muduo::net::EventLoop* loop, const muduo::string& ip, uint16_t port);
  ~Hiredis();

  const muduo::net::InetAddress& serverAddress() const { return serverAddr_; }
  muduo::net::Channel* getChannel() const { return get_pointer(channel_); }

  ConnectCallback connectCallback() const { return connectCb_; }
  DisconnectCallback disconnectCallback() const { return disconnectCb_; }
  CommandCallback commandCallback() const { return commandCb_; }

  void setConnectCallback(ConnectCallback cb)
  {
    connectCb_ = cb;
  }

  void setDisconnectCallback(DisconnectCallback cb)
  {
    disconnectCb_ = cb;
  }

  void connect();
  void setChannel();
  void removeChannel();

  // command
  int command(CommandCallback cb, const muduo::StringPiece& cmd);
  static void commandCallback(redisAsyncContext* ac, void*, void*);

  int ping();
  void pingCallback(redisAsyncContext*ac, redisReply* reply, void* privdata);

 private:
  void handleRead(muduo::Timestamp receiveTime);
  void handleWrite();

  static void connectCallback(const redisAsyncContext* ac, int status);
  static void disconnectCallback(const redisAsyncContext* ac, int status);

  static void addRead(void *privdata);
  static void delRead(void *privdata);
  static void addWrite(void *privdata);
  static void delWrite(void *privdata);
  static void cleanup(void *privdata);

 private:
  muduo::net::EventLoop* loop_;
  const muduo::net::InetAddress serverAddr_;
  redisAsyncContext *context_;
  boost::shared_ptr<muduo::net::Channel> channel_;
  ConnectCallback connectCb_;
  DisconnectCallback disconnectCb_;
  CommandCallback commandCb_;
};

}

#endif  // MUDUO_EXAMPLES_HIREDIS_HIREDIS_H
