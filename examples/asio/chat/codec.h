#ifndef MUDUO_EXAMPLES_ASIO_CHAT_CODEC_H
#define MUDUO_EXAMPLES_ASIO_CHAT_CODEC_H

#include <muduo/base/Logging.h>
#include <muduo/net/Buffer.h>
#include <muduo/net/SocketsOps.h>
#include <muduo/net/TcpConnection.h>

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>

using muduo::Logger;

class LengthHeaderCodec : boost::noncopyable
{
 public:
  typedef boost::function<void (const muduo::net::TcpConnectionPtr&,
                                const muduo::string& message,
                                muduo::Timestamp)> StringMessageCallback;

  explicit LengthHeaderCodec(const StringMessageCallback& cb)
    : messageCallback_(cb)
  {
  }

  void onMessage(const muduo::net::TcpConnectionPtr& conn,
                 muduo::net::Buffer* buf,
                 muduo::Timestamp receiveTime)
  {
    muduo::Timestamp& receiveTime_ = boost::any_cast<muduo::Timestamp&>(conn->getContext());
    if (!receiveTime_.valid())
    {
      receiveTime_ = receiveTime;
    }

    if (buf->readableBytes() >= kHeaderLen)
    {
      const void* data = buf->peek();
      int32_t tmp = *static_cast<const int32_t*>(data);
      int32_t len = muduo::net::sockets::networkToHost32(tmp);
      if (len > 65536 || len < 0)
      {
        LOG_ERROR << "Invalid length " << len;
        conn->shutdown();
      }
      else if (buf->readableBytes() >= len + kHeaderLen)
      {
        buf->retrieve(kHeaderLen);
        muduo::string message(buf->peek(), len);
        buf->retrieve(len);
        messageCallback_(conn, message, receiveTime_);
        receiveTime_ = muduo::Timestamp::invalid();
      }
    }
  }

  void send(muduo::net::TcpConnection* conn, const muduo::string& message)
  {
    muduo::net::Buffer buf;
    buf.append(message.data(), message.size());
    int32_t len = muduo::net::sockets::hostToNetwork32(static_cast<int32_t>(message.size()));
    buf.prepend(&len, sizeof len);
    conn->send(&buf);
  }

 private:
  StringMessageCallback messageCallback_;
  const static size_t kHeaderLen = sizeof(int32_t);
};

#endif  // MUDUO_EXAMPLES_ASIO_CHAT_CODEC_H
