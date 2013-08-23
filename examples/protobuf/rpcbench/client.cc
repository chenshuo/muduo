#include <examples/protobuf/rpcbench/echo.pb.h>

#include <muduo/base/CountDownLatch.h>
#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/EventLoopThreadPool.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/TcpClient.h>
#include <muduo/net/TcpConnection.h>
#include <muduo/net/protorpc/RpcChannel.h>

#include <boost/bind.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

#include <stdio.h>

using namespace muduo;
using namespace muduo::net;

static const int kRequests = 50000;

class RpcClient : boost::noncopyable
{
 public:

  RpcClient(EventLoop* loop,
            const InetAddress& serverAddr,
            CountDownLatch* allConnected,
            CountDownLatch* allFinished)
    : // loop_(loop),
      client_(loop, serverAddr, "RpcClient"),
      channel_(new RpcChannel),
      stub_(get_pointer(channel_)),
      allConnected_(allConnected),
      allFinished_(allFinished),
      count_(0)
  {
    client_.setConnectionCallback(
        boost::bind(&RpcClient::onConnection, this, _1));
    client_.setMessageCallback(
        boost::bind(&RpcChannel::onMessage, get_pointer(channel_), _1, _2, _3));
    // client_.enableRetry();
  }

  void connect()
  {
    client_.connect();
  }

  void sendRequest()
  {
    echo::EchoRequest request;
    request.set_payload("001010");
    echo::EchoResponse* response = new echo::EchoResponse;
    stub_.Echo(NULL, &request, response, NewCallback(this, &RpcClient::replied, response));
  }

 private:
  void onConnection(const TcpConnectionPtr& conn)
  {
    if (conn->connected())
    {
      //channel_.reset(new RpcChannel(conn));
      conn->setTcpNoDelay(true);
      channel_->setConnection(conn);
      allConnected_->countDown();
    }
  }

  void replied(echo::EchoResponse* resp)
  {
    // LOG_INFO << "replied:\n" << resp->DebugString().c_str();
    // loop_->quit();
    ++count_;
    if (count_ < kRequests)
    {
      sendRequest();
    }
    else
    {
      LOG_INFO << "RpcClient " << this << " finished";
      allFinished_->countDown();
    }
  }

  // EventLoop* loop_;
  TcpClient client_;
  RpcChannelPtr channel_;
  echo::EchoService::Stub stub_;
  CountDownLatch* allConnected_;
  CountDownLatch* allFinished_;
  int count_;
};

int main(int argc, char* argv[])
{
  LOG_INFO << "pid = " << getpid();
  if (argc > 1)
  {
    int nClients = 1;

    if (argc > 2)
    {
      nClients = atoi(argv[2]);
    }

    int nThreads = 1;

    if (argc > 3)
    {
      nThreads = atoi(argv[3]);
    }

    CountDownLatch allConnected(nClients);
    CountDownLatch allFinished(nClients);

    EventLoop loop;
    EventLoopThreadPool pool(&loop);
    pool.setThreadNum(nThreads);
    pool.start();
    InetAddress serverAddr(argv[1], 8888);

    boost::ptr_vector<RpcClient> clients;
    for (int i = 0; i < nClients; ++i)
    {
      clients.push_back(new RpcClient(pool.getNextLoop(), serverAddr, &allConnected, &allFinished));
      clients.back().connect();
    }
    allConnected.wait();
    Timestamp start(Timestamp::now());
    LOG_INFO << "all connected";
    for (int i = 0; i < nClients; ++i)
    {
      clients[i].sendRequest();
    }
    allFinished.wait();
    Timestamp end(Timestamp::now());
    LOG_INFO << "all finished";
    double seconds = timeDifference(end, start);
    printf("%f seconds\n", seconds);
    printf("%.1f calls per second\n", nClients * kRequests / seconds);

    exit(0);
  }
  else
  {
    printf("Usage: %s host_ip numClients [numThreads]\n", argv[0]);
  }
}

