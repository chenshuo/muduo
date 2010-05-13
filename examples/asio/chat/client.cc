#include "codec.h"

#include <muduo/base/Logging.h>
#include <muduo/base/Mutex.h>
#include <muduo/net/EventLoopThread.h>
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
      client_(loop, listenAddr, "ChatClient"),
      codec_(boost::bind(&ChatClient::onStringMessage, this, _1, _2, _3))
  {
    client_.setConnectionCallback(
        boost::bind(&ChatClient::onConnection, this, _1));
    client_.setMessageCallback(
        boost::bind(&LengthHeaderCodec::onMessage, &codec_, _1, _2, _3));
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
      codec_.send(get_pointer(connection_), message);
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
      conn->setContext(Timestamp());
      connection_ = conn;
    }
    else
    {
      connection_.reset();
    }
  }

  void onStringMessage(const TcpConnectionPtr&,
                       const string& message,
                       Timestamp)
  {
    printf("<<< %s\n", message.c_str());
  }

  EventLoop* loop_;
  TcpClient client_;
  LengthHeaderCodec codec_;
  MutexLock mutex_;
  TcpConnectionPtr connection_;
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

