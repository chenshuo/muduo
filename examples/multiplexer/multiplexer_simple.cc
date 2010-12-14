#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/TcpClient.h>
#include <muduo/net/TcpServer.h>

#include <boost/bind.hpp>

#include <queue>
#include <utility>

#include <mcheck.h>
#include <stdio.h>
#include <unistd.h>

using namespace muduo;
using namespace muduo::net;

const int kMaxConns = 65535;
const size_t kMaxPacketLen = 255;
const size_t kHeaderLen = 3;

const uint16_t kClientPort = 3333;
const char* kBackendIp = "127.0.0.1";
const uint16_t kBackendPort = 9999;

class MultiplexServer
{
 public:
  MultiplexServer(EventLoop* loop, const InetAddress& listenAddr, const InetAddress& backendAddr)
    : loop_(loop),
      server_(loop, listenAddr, "MultiplexServer"),
      backend_(loop, backendAddr, "MultiplexBackend")
  {
    server_.setConnectionCallback(
        boost::bind(&MultiplexServer::onClientConnection, this, _1));
    server_.setMessageCallback(
        boost::bind(&MultiplexServer::onClientMessage, this, _1, _2, _3));
    backend_.setConnectionCallback(
        boost::bind(&MultiplexServer::onBackendConnection, this, _1));
    backend_.setMessageCallback(
        boost::bind(&MultiplexServer::onBackendMessage, this, _1, _2, _3));
    backend_.enableRetry();
  }

  void start()
  {
    backend_.connect();
    server_.start();
  }

 private:
  void sendBackendPacket(int id, Buffer* buf)
  {
    size_t len = buf->readableBytes();
    assert(len <= kMaxPacketLen);
    uint8_t header[kHeaderLen] = {
      static_cast<uint8_t>(len),
      static_cast<uint8_t>(id & 0xFF),
      static_cast<uint8_t>((id & 0xFF00) >> 8)
    };
    buf->prepend(header, kHeaderLen);
    if (backendConn_) {
      backendConn_->send(buf);
    }
  }

  void sendBackendString(int id, const string& msg)
  {
    assert(msg.size() <= kMaxPacketLen);
    Buffer buf;
    buf.append(msg);
    sendBackendPacket(id, &buf);
  }

  void sendBackendBuffer(int id, Buffer* buf)
  {
    while (buf->readableBytes() > kMaxPacketLen) {
      string msg(buf->peek(), kMaxPacketLen);
      buf->retrieve(kMaxPacketLen);
      sendBackendString(id, msg);
    }
    if (buf->readableBytes() > 0) {
      sendBackendPacket(id, buf);
    }
  }

  void sendToClient(Buffer* buf)
  {
    while (buf->readableBytes() > kHeaderLen) {
      size_t len = static_cast<uint8_t>(*buf->peek());
      if (buf->readableBytes() < len + kHeaderLen) {
        break;
      } else {
        int id = static_cast<uint8_t>(buf->peek()[1]);
        id |= (static_cast<uint8_t>(buf->peek()[2]) << 8);

        if (id != 0) {
          std::map<int, TcpConnectionPtr>::iterator it = clientConns_.find(id);
          if (it != clientConns_.end()) {
            it->second->send(buf->peek() + kHeaderLen, len);
          }
        } else {
          // TODO: act on backend's command
        }
        buf->retrieve(len + kHeaderLen);
      }
    }
  }

  void onClientConnection(const TcpConnectionPtr& conn)
  {
    LOG_TRACE << "Client " << conn->peerAddress().toHostPort() << " -> "
        << conn->localAddress().toHostPort() << " is "
        << (conn->connected() ? "UP" : "DOWN");
    if (conn->connected()) {
      int id = -1;
      if (!availIds_.empty()) {
        id = availIds_.front();
        availIds_.pop();
        clientConns_[id] = conn;
      }

      if (id <= 0) {
        conn->shutdown();
      } else {
        conn->setContext(id);
        char buf[256];
        snprintf(buf, sizeof(buf), "CONN %d FROM %s IS UP\r\n", id,
                 conn->peerAddress().toHostPort().c_str());
        sendBackendString(0, buf);
      }
    } else {
      if (!conn->getContext().empty()) {
        int id = boost::any_cast<int>(conn->getContext());
        assert(id > 0 && id <= kMaxConns);
        char buf[256];
        snprintf(buf, sizeof(buf), "CONN %d FROM %s IS DOWN\r\n",
                 id, conn->peerAddress().toHostPort().c_str());
        sendBackendString(0, buf);

        if (backendConn_) {
          availIds_.push(id);
          clientConns_.erase(id);
        } else {
          assert(availIds_.empty());
          assert(clientConns_.empty());
        }
      }
    }
  }

  void onClientMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp)
  {
    int id = boost::any_cast<int>(conn->getContext());
    sendBackendBuffer(id, buf);
  }

  void onBackendConnection(const TcpConnectionPtr& conn)
  {
    LOG_TRACE << "Backend " << conn->localAddress().toHostPort() << " -> "
              << conn->peerAddress().toHostPort() << " is "
              << (conn->connected() ? "UP" : "DOWN");
    std::vector<TcpConnectionPtr> connsToDestroy;
    if (conn->connected()) {
      backendConn_ = conn;
      assert(availIds_.empty());
      for (int i = 1; i <= kMaxConns; ++i) {
        availIds_.push(i);
      }
    }
    else
    {
      backendConn_.reset();
      connsToDestroy.reserve(clientConns_.size());
      for (std::map<int, TcpConnectionPtr>::iterator it = clientConns_.begin();
          it != clientConns_.end();
          ++it) {
        connsToDestroy.push_back(it->second);
      }
      clientConns_.clear();
      while (!availIds_.empty()) {
        availIds_.pop();
      }
    }

    for (std::vector<TcpConnectionPtr>::iterator it = connsToDestroy.begin();
        it != connsToDestroy.end();
        ++it) {
      (*it)->shutdown();
    }
  }

  void onBackendMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp)
  {
    sendToClient(buf);
  }

  EventLoop* loop_;
  TcpServer server_;
  TcpClient backend_;
  // MutexLock mutex_;
  TcpConnectionPtr backendConn_;
  std::map<int, TcpConnectionPtr> clientConns_;
  std::queue<int> availIds_;
};

int main(int argc, char* argv[])
{
  LOG_INFO << "pid = " << getpid();
  EventLoop loop;
  InetAddress listenAddr(kClientPort);
  InetAddress backendAddr(kBackendIp, kBackendPort);
  MultiplexServer server(&loop, listenAddr, backendAddr);

  server.start();

  loop.loop();
}

