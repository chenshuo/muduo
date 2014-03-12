#include <muduo/base/Logging.h>
#include <muduo/base/ThreadLocal.h>
#include <muduo/net/EventLoop.h>
//#include <muduo/net/EventLoopThread.h>
#include <muduo/net/EventLoopThreadPool.h>
#include <muduo/net/TcpClient.h>
#include <muduo/net/TcpServer.h>
//#include <muduo/net/inspect/Inspector.h>
#include <muduo/net/protorpc/RpcCodec.h>
#include <muduo/net/protorpc/rpc.pb.h>

#include <boost/bind.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

#include <endian.h>
#include <stdio.h>

using namespace muduo;
using namespace muduo::net;

struct RawMessage
{
  RawMessage(StringPiece m)
    : message_(m), id_(0), loc_(NULL)
  { }

  uint64_t id() const { return id_; }
  void set_id(uint64_t x) { id_ = x; }

  bool parse(const string& tag)
  {
    const char* const body = message_.data() + ProtobufCodecLite::kHeaderLen;
    const int bodylen = message_.size() - ProtobufCodecLite::kHeaderLen;
    const int taglen = static_cast<int>(tag.size());
    if (ProtobufCodecLite::validateChecksum(body, bodylen)
        && (memcmp(body, tag.data(), tag.size()) == 0)
        && (bodylen >= taglen + 3 + 8))
    {
      const char* const p = body + taglen;
      uint8_t type = *(p+1);

      if (*p == 0x08 && (type == 0x01 || type == 0x02) && *(p+2) == 0x11)
      {
        uint64_t x = 0;
        memcpy(&x, p+3, sizeof(x));
        set_id(le64toh(x));
        loc_ = p+3;
        return true;
      }
    }
    return false;
  }

  void updateId()
  {
    uint64_t le64 = htole64(id_);
    memcpy(const_cast<void*>(loc_), &le64, sizeof(le64));

    const char* body = message_.data() + ProtobufCodecLite::kHeaderLen;
    int bodylen = message_.size() - ProtobufCodecLite::kHeaderLen;
    int32_t checkSum = ProtobufCodecLite::checksum(body, bodylen - ProtobufCodecLite::kChecksumLen);
    int32_t be32 = sockets::hostToNetwork32(checkSum);
    memcpy(const_cast<char*>(body + bodylen - ProtobufCodecLite::kChecksumLen), &be32, sizeof(be32));
  }

  StringPiece message_;

 private:
  uint64_t id_;
  const void* loc_;
};

class BackendSession : boost::noncopyable
{
 public:
  BackendSession(EventLoop* loop, const InetAddress& backendAddr, const string& name)
    : loop_(loop),
      client_(loop, backendAddr, name),
      codec_(boost::bind(&BackendSession::onRpcMessage, this, _1, _2, _3),
             boost::bind(&BackendSession::onRawMessage, this, _1, _2, _3)),
      nextId_(0)
  {
    client_.setConnectionCallback(
        boost::bind(&BackendSession::onConnection, this, _1));
    client_.setMessageCallback(
        boost::bind(&RpcCodec::onMessage, &codec_, _1, _2, _3));
    client_.enableRetry();
  }

  void connect()
  {
    client_.connect();
  }

  // FIXME: add health check
  template<typename MSG>
  bool send(MSG& msg, const TcpConnectionPtr& clientConn)
  {
    loop_->assertInLoopThread();
    if (conn_)
    {
      uint64_t id = ++nextId_;
      Request r = { msg.id(), clientConn };
      assert(outstandings_.find(id) == outstandings_.end());
      outstandings_[id] = r;
      msg.set_id(id);
      sendTo(conn_, msg);
      // LOG_DEBUG << "forward " << r.origId << " from " << clientConn->name()
      //           << " as " << id << " to " << conn_->name();
      return true;
    }
    else
      return false;
  }

 private:
  void sendTo(const TcpConnectionPtr& conn, const RpcMessage& msg)
  {
    codec_.send(conn, msg);
  }

  void sendTo(const TcpConnectionPtr& conn, RawMessage& msg)
  {
    msg.updateId();
    conn->send(msg.message_);
  }

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
    onMessageT(*msg);
  }

  bool onRawMessage(const TcpConnectionPtr&,
                    StringPiece message,
                    Timestamp)
  {
    RawMessage raw(message);
    if (raw.parse(codec_.tag()))
    {
      onMessageT(raw);
      return false;
    }
    else
      return true; // try normal rpc message callback
  }

  template<typename MSG>
  void onMessageT(MSG& msg)
  {
    loop_->assertInLoopThread();
    std::map<uint64_t, Request>::iterator it = outstandings_.find(msg.id());
    if (it != outstandings_.end())
    {
      uint64_t origId = it->second.origId;
      TcpConnectionPtr clientConn = it->second.clientConn.lock();
      outstandings_.erase(it);

      if (clientConn)
      {
        // LOG_DEBUG << "send back " << origId << " of " << clientConn->name()
        //           << " using " << msg.id() << " from " << conn_->name();
        msg.set_id(origId);
        sendTo(clientConn, msg);
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
    boost::weak_ptr<TcpConnection> clientConn;
  };

  EventLoop* loop_;
  TcpClient client_;
  RpcCodec codec_;
  TcpConnectionPtr conn_;
  uint64_t nextId_;
  std::map<uint64_t, Request> outstandings_;
};

class Balancer : boost::noncopyable
{
 public:
  Balancer(EventLoop* loop,
           const InetAddress& listenAddr,
           const string& name,
           const std::vector<InetAddress>& backends)
    : loop_(loop),
      server_(loop, listenAddr, name),
      codec_(boost::bind(&Balancer::onRpcMessage, this, _1, _2, _3),
             boost::bind(&Balancer::onRawMessage, this, _1, _2, _3)),
      backends_(backends)
  {
    server_.setThreadInitCallback(
        boost::bind(&Balancer::initPerThread, this, _1));
    server_.setConnectionCallback(
        boost::bind(&Balancer::onConnection, this, _1));
    server_.setMessageCallback(
        boost::bind(&RpcCodec::onMessage, &codec_, _1, _2, _3));
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
    boost::ptr_vector<BackendSession> backends;
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
      t.backends.push_back(new BackendSession(ioLoop, backends_[i], buf));
      t.backends.back().connect();
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

  bool onRawMessage(const TcpConnectionPtr& conn,
                    StringPiece message,
                    Timestamp)
  {
    RawMessage raw(message);
    if (raw.parse(codec_.tag()))
    {
      onMessageT(conn, raw);
      return false;
    }
    else
      return true; // try normal rpc message callback
  }

  void onRpcMessage(const TcpConnectionPtr& conn,
                    const RpcMessagePtr& msg,
                    Timestamp)
  {
    onMessageT(conn, *msg);
  }

  template<typename MSG>
  bool onMessageT(const TcpConnectionPtr& conn, MSG& msg)
  {
    PerThread& t = t_backends_.value();
    bool succeed = false;
    for (size_t i = 0; i < t.backends.size() && !succeed; ++i)
    {
      succeed = t.backends[t.current].send(msg, conn);
      t.current = (t.current+1) % t.backends.size();
    }
    if (!succeed)
    {
      // FIXME: no backend available
    }
    return succeed;
  }

  EventLoop* loop_;
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

    // EventLoopThread inspectThread;
    // new Inspector(inspectThread.startLoop(), InetAddress(8080), "rpcbalancer");
    EventLoop loop;
    Balancer balancer(&loop, listenAddr, "RpcBalancer", backends);
    balancer.setThreadNum(4);
    balancer.start();
    loop.loop();
  }
  google::protobuf::ShutdownProtobufLibrary();
}

