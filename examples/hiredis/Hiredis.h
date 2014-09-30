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

#include <hiredis/hiredis.h>

struct redisAsyncContext;

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

class Hiredis : public boost::enable_shared_from_this<Hiredis>,
                boost::noncopyable
{
 public:
  typedef boost::function<void(Hiredis*, int)> ConnectCallback;
  typedef boost::function<void(Hiredis*, int)> DisconnectCallback;
  typedef boost::function<void(Hiredis*, redisReply*)> CommandCallback;

  Hiredis(muduo::net::EventLoop* loop, const muduo::net::InetAddress& serverAddr);
  ~Hiredis();

  const muduo::net::InetAddress& serverAddress() const { return serverAddr_; }
  // redisAsyncContext* context() { return context_; }
  bool connected() const;
  const char* errstr() const;

  void setConnectCallback(const ConnectCallback& cb) { connectCb_ = cb; }
  void setDisconnectCallback(const DisconnectCallback& cb) { disconnectCb_ = cb; }

  void connect();
  void disconnect();  // FIXME: implement this with redisAsyncDisconnect

  int command(const CommandCallback& cb, muduo::StringArg cmd, ...);

  int ping();

 private:
  void handleRead(muduo::Timestamp receiveTime);
  void handleWrite();

  int fd() const;
  void logConnection(bool up) const;
  void setChannel();
  void removeChannel();

  void connectCallback(int status);
  void disconnectCallback(int status);
  void commandCallback(redisReply* reply, CommandCallback* privdata);

  static Hiredis* getHiredis(const redisAsyncContext* ac);

  static void connectCallback(const redisAsyncContext* ac, int status);
  static void disconnectCallback(const redisAsyncContext* ac, int status);
  // command callback
  static void commandCallback(redisAsyncContext* ac, void*, void*);

  static void addRead(void* privdata);
  static void delRead(void* privdata);
  static void addWrite(void* privdata);
  static void delWrite(void* privdata);
  static void cleanup(void* privdata);

  void pingCallback(Hiredis* me, redisReply* reply);

 private:
  muduo::net::EventLoop* loop_;
  const muduo::net::InetAddress serverAddr_;
  redisAsyncContext* context_;
  boost::shared_ptr<muduo::net::Channel> channel_;
  ConnectCallback connectCb_;
  DisconnectCallback disconnectCb_;
};

}

#endif  // MUDUO_EXAMPLES_HIREDIS_HIREDIS_H
