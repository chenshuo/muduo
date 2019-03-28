#include "examples/asio/chat/codec.h"

#include "muduo/base/Atomic.h"
#include "muduo/base/Logging.h"
#include "muduo/base/Mutex.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/EventLoopThreadPool.h"
#include "muduo/net/TcpClient.h"

#include <stdio.h>
#include <unistd.h>

using namespace muduo;
using namespace muduo::net;

int g_connections = 0;
AtomicInt32 g_aliveConnections;
AtomicInt32 g_messagesReceived;
Timestamp g_startTime;
std::vector<Timestamp> g_receiveTime;
EventLoop* g_loop;
std::function<void()> g_statistic;

class ChatClient : noncopyable
{
 public:
  ChatClient(EventLoop* loop, const InetAddress& serverAddr)
    : loop_(loop),
      client_(loop, serverAddr, "LoadTestClient"),
      codec_(std::bind(&ChatClient::onStringMessage, this, _1, _2, _3))
  {
    client_.setConnectionCallback(
        std::bind(&ChatClient::onConnection, this, _1));
    client_.setMessageCallback(
        std::bind(&LengthHeaderCodec::onMessage, &codec_, _1, _2, _3));
    //client_.enableRetry();
  }

  void connect()
  {
    client_.connect();
  }

  void disconnect()
  {
    // client_.disconnect();
  }

  Timestamp receiveTime() const { return receiveTime_; }

 private:
  void onConnection(const TcpConnectionPtr& conn)
  {
    LOG_INFO << conn->localAddress().toIpPort() << " -> "
             << conn->peerAddress().toIpPort() << " is "
             << (conn->connected() ? "UP" : "DOWN");

    if (conn->connected())
    {
      connection_ = conn;
      if (g_aliveConnections.incrementAndGet() == g_connections)
      {
        LOG_INFO << "all connected";
        loop_->runAfter(10.0, std::bind(&ChatClient::send, this));
      }
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
    // printf("<<< %s\n", message.c_str());
    receiveTime_ = loop_->pollReturnTime();
    int received = g_messagesReceived.incrementAndGet();
    if (received == g_connections)
    {
      Timestamp endTime = Timestamp::now();
      LOG_INFO << "all received " << g_connections << " in "
               << timeDifference(endTime, g_startTime);
      g_loop->queueInLoop(g_statistic);
    }
    else if (received % 1000 == 0)
    {
      LOG_DEBUG << received;
    }
  }

  void send()
  {
    g_startTime = Timestamp::now();
    codec_.send(get_pointer(connection_), "hello");
    LOG_DEBUG << "sent";
  }

  EventLoop* loop_;
  TcpClient client_;
  LengthHeaderCodec codec_;
  TcpConnectionPtr connection_;
  Timestamp receiveTime_;
};

void statistic(const std::vector<std::unique_ptr<ChatClient>>& clients)
{
  LOG_INFO << "statistic " << clients.size();
  std::vector<double> seconds(clients.size());
  for (size_t i = 0; i < clients.size(); ++i)
  {
    seconds[i] = timeDifference(clients[i]->receiveTime(), g_startTime);
  }

  std::sort(seconds.begin(), seconds.end());
  for (size_t i = 0; i < clients.size(); i += std::max(static_cast<size_t>(1), clients.size()/20))
  {
    printf("%6zd%% %.6f\n", i*100/clients.size(), seconds[i]);
  }
  if (clients.size() >= 100)
  {
    printf("%6d%% %.6f\n", 99, seconds[clients.size() - clients.size()/100]);
  }
  printf("%6d%% %.6f\n", 100, seconds.back());
}

int main(int argc, char* argv[])
{
  LOG_INFO << "pid = " << getpid();
  if (argc > 3)
  {
    uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
    InetAddress serverAddr(argv[1], port);
    g_connections = atoi(argv[3]);
    int threads = 0;
    if (argc > 4)
    {
      threads = atoi(argv[4]);
    }

    EventLoop loop;
    g_loop = &loop;
    EventLoopThreadPool loopPool(&loop, "chat-loadtest");
    loopPool.setThreadNum(threads);
    loopPool.start();

    g_receiveTime.reserve(g_connections);
    std::vector<std::unique_ptr<ChatClient>> clients(g_connections);
    g_statistic = std::bind(statistic, std::ref(clients));

    for (int i = 0; i < g_connections; ++i)
    {
      clients[i].reset(new ChatClient(loopPool.getNextLoop(), serverAddr));
      clients[i]->connect();
      usleep(200);
    }

    loop.loop();
    // client.disconnect();
  }
  else
  {
    printf("Usage: %s host_ip port connections [threads]\n", argv[0]);
  }
}


