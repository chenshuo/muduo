#include "muduo/net/TcpClient.h"

#include "muduo/base/Logging.h"
#include "muduo/base/Thread.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/InetAddress.h"

#include <utility>

#include <stdio.h>
#include <unistd.h>

using namespace muduo;
using namespace muduo::net;

class UptimeClient : noncopyable
{
 public:
  UptimeClient(EventLoop* loop, const InetAddress& listenAddr)
    : client_(loop, listenAddr, "UptimeClient")
  {
    client_.setConnectionCallback(
        std::bind(&UptimeClient::onConnection, this, _1));
    client_.setMessageCallback(
        std::bind(&UptimeClient::onMessage, this, _1, _2, _3));
    //client_.enableRetry();
  }

  void connect()
  {
    client_.connect();
  }

 private:
  void onConnection(const TcpConnectionPtr& conn)
  {
    LOG_TRACE << conn->localAddress().toIpPort() << " -> "
        << conn->peerAddress().toIpPort() << " is "
        << (conn->connected() ? "UP" : "DOWN");
  }

  void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time)
  {
  }

  TcpClient client_;
};

int main(int argc, char* argv[])
{
  LOG_INFO << "pid = " << getpid() << ", tid = " << CurrentThread::tid();
  if (argc > 2)
  {
    EventLoop loop;
    uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
    InetAddress serverAddr(argv[1], port);

    UptimeClient client(&loop, serverAddr);
    client.connect();
    loop.loop();
  }
  else
  {
    printf("Usage: %s host_ip port\n", argv[0]);
  }
}

