#include "muduo/base/Logging.h"
#include "muduo/base/ThreadLocal.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/EventLoopThreadPool.h"
#include "muduo/net/TcpClient.h"
#include "muduo/net/TcpServer.h"
#include "muduo/net/protorpc/RpcCodec.h"
#include "muduo/net/protorpc/rpc.pb.h"

#include <stdio.h>
#include <unistd.h>

using namespace muduo;
using namespace muduo::net;

class BackendSession : noncopyable
{
 public:
  BackendSession(EventLoop* loop, const InetAddress& backendAddr, const string& name)
    : loop_(loop),
      client_(loop, backendAddr, name),
      codec_(std::bind(&BackendSession::onRpcMessage, this, _1, _2, _3)),
      nextId_(0)
  {
    client_.setConnectionCallback(
        std::bind(&BackendSession::onConnection, this, _1));
    client_.setMessageCallback(
        std::bind(&RpcCodec::onMessage, &codec_, _1, _2, _3));
    client_.enableRetry();
  }

  void connect()
  {
    client_.connect();
  }

  // FIXME: add health check
  bool send(RpcMessage& msg, const TcpConnectionPtr& clientConn)
  {
    loop_->assertInLoopThread();
    if (conn_)
    {
      uint64_t id = ++nextId_;
      Request r = { msg.id(), clientConn };
      assert(outstandings_.find(id) == outstandings_.end());
      outstandings_[id] = r;
      msg.set_id(id);
      codec_.send(conn_, msg);
      // LOG_DEBUG << "forward " << r.origId << " from " << clientConn->name()
      //           << " as " << id << " to " << conn_->name();
      return true;
    }
    else
      return false;
  }

 private:
  void onConnection(const TcpConnectionPtr& conn)
  {
    loop_->assertInLoopThread();
    LOG_INFO << "Backend "
             << conn->localAddress().toIpPort() << " -> "
             << conn->peerAddress().toIpPort() << " is "
             << (conn->connected() ? "UP" : "DOWN");
    if (conn->connected())
    {
      conn_ = conn;
    }
    else
    {
      conn_.reset();
      // FIXME: reject pending
    }
  }

  void onRpcMessage(const TcpConnectionPtr&,
                    const RpcMessagePtr& msg,
                    Timestamp)
  {
    loop_->assertInLoopThread();
    std::map<uint64_t, Request>::iterator it = outstandings_.find(msg->id());
    if (it != outstandings_.end())
    {
      uint64_t origId = it->second.origId;
      TcpConnectionPtr clientConn = it->second.clientConn.lock();
      outstandings_.erase(it);

      if (clientConn)
      {
        // LOG_DEBUG << "send back " << origId << " of " << clientConn->name()
        //           << " using " << msg.id() << " from " << conn_->name();
        msg->set_id(origId);
        codec_.send(clientConn, *msg);
      }
    }
    else
    {
      // LOG_ERROR
    }
  }

  struct Request
  {
    uint64_t origId;
    std::weak_ptr<TcpConnection> clientConn;
  };

  EventLoop* loop_;
  TcpClient client_;
  RpcCodec codec_;
  TcpConnectionPtr conn_;
  uint64_t nextId_;
  std::map<uint64_t, Request> outstandings_;
};

class Balancer : noncopyable
{
 public:
  Balancer(EventLoop* loop,
           const InetAddress& listenAddr,
           const string& name,
           const std::vector<InetAddress>& backends)
    : server_(loop, listenAddr, name),
      codec_(std::bind(&Balancer::onRpcMessage, this, _1, _2, _3)),
      backends_(backends)
  {
    server_.setThreadInitCallback(
        std::bind(&Balancer::initPerThread, this, _1));
    server_.setConnectionCallback(
        std::bind(&Balancer::onConnection, this, _1));
    server_.setMessageCallback(
        std::bind(&RpcCodec::onMessage, &codec_, _1, _2, _3));
  }

  ~Balancer()
  {
  }

  void setThreadNum(int numThreads)
  {
    server_.setThreadNum(numThreads);
  }

  void start()
  {
    server_.start();
  }

 private:
  struct PerThread
  {
    size_t current;
    std::vector<std::unique_ptr<BackendSession>> backends;
    PerThread() : current(0) { }
  };

  void initPerThread(EventLoop* ioLoop)
  {
    int count = threadCount_.getAndAdd(1);
    LOG_INFO << "IO thread " << count;
    PerThread& t = t_backends_.value();
    t.current = count % backends_.size();

    for (size_t i = 0; i < backends_.size(); ++i)
    {
      char buf[32];
      snprintf(buf, sizeof buf, "%s#%d", backends_[i].toIpPort().c_str(), count);
      t.backends.emplace_back(new BackendSession(ioLoop, backends_[i], buf));
      t.backends.back()->connect();
    }
  }

  void onConnection(const TcpConnectionPtr& conn)
  {
    LOG_INFO << "Client "
             << conn->peerAddress().toIpPort() << " -> "
             << conn->localAddress().toIpPort() << " is "
             << (conn->connected() ? "UP" : "DOWN");
    if (!conn->connected())
    {
      // FIXME: cancel outstanding calls, otherwise, memory leak
    }
  }

  void onRpcMessage(const TcpConnectionPtr& conn,
                    const RpcMessagePtr& msg,
                    Timestamp)
  {
    PerThread& t = t_backends_.value();
    bool succeed = false;
    for (size_t i = 0; i < t.backends.size() && !succeed; ++i)
    {
      succeed = t.backends[t.current]->send(*msg, conn);
      t.current = (t.current+1) % t.backends.size();
    }
    if (!succeed)
    {
      // FIXME: no backend available
    }
  }

  TcpServer server_;
  RpcCodec codec_;
  std::vector<InetAddress> backends_;
  AtomicInt32 threadCount_;
  ThreadLocal<PerThread> t_backends_;
};

int main(int argc, char* argv[])
{
  LOG_INFO << "pid = " << getpid();
  if (argc < 3)
  {
    fprintf(stderr, "Usage: %s listen_port backend_ip:port [backend_ip:port]\n", argv[0]);
  }
  else
  {
    std::vector<InetAddress> backends;
    for (int i = 2; i < argc; ++i)
    {
      string hostport = argv[i];
      size_t colon = hostport.find(':');
      if (colon != string::npos)
      {
        string ip = hostport.substr(0, colon);
        uint16_t port = static_cast<uint16_t>(atoi(hostport.c_str()+colon+1));
        backends.push_back(InetAddress(ip, port));
      }
      else
      {
        fprintf(stderr, "invalid backend address %s\n", argv[i]);
        return 1;
      }
    }
    uint16_t port = static_cast<uint16_t>(atoi(argv[1]));
    InetAddress listenAddr(port);

    EventLoop loop;
    Balancer balancer(&loop, listenAddr, "RpcBalancer", backends);
    balancer.setThreadNum(4);
    balancer.start();
    loop.loop();
  }
  google::protobuf::ShutdownProtobufLibrary();
}

