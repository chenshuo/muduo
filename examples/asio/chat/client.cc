#include <muduo/base/Logging.h>
#include <muduo/base/Mutex.h>
#include <muduo/net/EventLoopThread.h>
#include <muduo/net/SocketsOps.h>
#include <muduo/net/TcpClient.h>

#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>

#include <iostream>
#include <stdio.h>

using namespace muduo;
using namespace muduo::net;

class ChatClient : boost::noncopyable
{
 public:
  ChatClient(EventLoop* loop, const InetAddress& listenAddr)
    : loop_(loop),
      client_(loop, listenAddr, "ChatClient")
  {
    client_.setConnectionCallback(
        boost::bind(&ChatClient::onConnection, this, _1));
    client_.setMessageCallback(
        boost::bind(&ChatClient::onMessage, this, _1, _2, _3));
    client_.enableRetry();
  }

  void connect()
  {
    client_.connect();
  }

  void disconnect()
  {
    // client_.disconnect();
  }

  void write(const string& message)
  {
    MutexLockGuard lock(mutex_);
    if (connection_)
    {
      Buffer buf;
      buf.append(message.data(), message.size());
      int32_t len = sockets::hostToNetwork32(static_cast<int32_t>(message.size()));
      buf.prepend(&len, sizeof len);
      connection_->send(&buf);
    }
  }

 private:
  void onConnection(const TcpConnectionPtr& conn)
  {
    LOG_INFO << conn->localAddress().toHostPort() << " -> "
        << conn->peerAddress().toHostPort() << " is "
        << (conn->connected() ? "UP" : "DOWN");

    MutexLockGuard lock(mutex_);
    if (conn->connected())
    {
      connection_ = conn;
    }
    else
    {
      connection_.reset();
    }
  }

  void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp receiveTime)
  {
    if (buf->readableBytes() >= kHeaderLen)
    {
      const void* data = buf->peek();
      int32_t tmp = *static_cast<const int32_t*>(data);
      int32_t len = sockets::networkToHost32(tmp);
      if (len > 65536 || len < 0)
      {
        LOG_ERROR << "Invalid length " << len;
        conn->shutdown();
      }
      else
      {
        if (buf->readableBytes() >= len + kHeaderLen)
        {
          buf->retrieve(kHeaderLen);
          string message(buf->peek(), len);
          buf->retrieve(len);
          printf("<<< %s\n", message.c_str());
        }
      }
    }
    else
    {
      LOG_INFO << conn->name() << " no enough data " << buf->readableBytes()
       << " at " << receiveTime.toFormattedString();
    }
  }

  EventLoop* loop_;
  TcpClient client_;
  MutexLock mutex_;
  TcpConnectionPtr connection_;
  const static size_t kHeaderLen = 4;
};

int main(int argc, char* argv[])
{
  LOG_INFO << "pid = " << getpid();
  if (argc > 2)
  {
    EventLoopThread loopThread;
    uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
    InetAddress serverAddr(argv[1], port);

    ChatClient client(loopThread.startLoop(), serverAddr);
    client.connect();
    std::string line;
    while (std::getline(std::cin, line))
    {
      string message(line.c_str());
      client.write(message);
    }
    client.disconnect();
  }
  else
  {
    printf("Usage: %s host_ip port\n", argv[0]);
  }
}

