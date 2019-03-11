#include "muduo/base/Logging.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/InetAddress.h"
#include "muduo/net/TcpClient.h"
#include "muduo/net/TcpServer.h"

#include <queue>
#include <utility>

#include <stdio.h>
#include <unistd.h>

using namespace muduo;
using namespace muduo::net;

const int kMaxConns = 10;  // 65535
const size_t kMaxPacketLen = 255;
const size_t kHeaderLen = 3;

const uint16_t kClientPort = 3333;
const char* backendIp = "127.0.0.1";
const uint16_t kBackendPort = 9999;

class MultiplexServer : noncopyable
{
 public:
  MultiplexServer(EventLoop* loop, const InetAddress& listenAddr, const InetAddress& backendAddr)
    : server_(loop, listenAddr, "MultiplexServer"),
      backend_(loop, backendAddr, "MultiplexBackend")
  {
    server_.setConnectionCallback(
        std::bind(&MultiplexServer::onClientConnection, this, _1));
    server_.setMessageCallback(
        std::bind(&MultiplexServer::onClientMessage, this, _1, _2, _3));
    backend_.setConnectionCallback(
        std::bind(&MultiplexServer::onBackendConnection, this, _1));
    backend_.setMessageCallback(
        std::bind(&MultiplexServer::onBackendMessage, this, _1, _2, _3));
    backend_.enableRetry();
  }

  void start()
  {
    backend_.connect();
    server_.start();
  }

 private:

  void onClientConnection(const TcpConnectionPtr& conn)
  {
    LOG_TRACE << "Client " << conn->peerAddress().toIpPort() << " -> "
        << conn->localAddress().toIpPort() << " is "
        << (conn->connected() ? "UP" : "DOWN");
    if (conn->connected())
    {
      int id = -1;
      if (!availIds_.empty())
      {
        id = availIds_.front();
        availIds_.pop();
        clientConns_[id] = conn;
      }

      if (id <= 0)
      {
        // no client id available
        conn->shutdown();
      }
      else
      {
        conn->setContext(id);
        char buf[256];
        snprintf(buf, sizeof(buf), "CONN %d FROM %s IS UP\r\n",
                 id, conn->peerAddress().toIpPort().c_str());
        sendBackendString(0, buf);
      }
    }
    else
    {
      if (!conn->getContext().empty())
      {
        int id = boost::any_cast<int>(conn->getContext());
        assert(id > 0 && id <= kMaxConns);
        char buf[256];
        snprintf(buf, sizeof(buf), "CONN %d FROM %s IS DOWN\r\n",
                 id, conn->peerAddress().toIpPort().c_str());
        sendBackendString(0, buf);

        if (backendConn_)
        {
          // put client id back for reusing
          availIds_.push(id);
          clientConns_.erase(id);
        }
        else
        {
          assert(availIds_.empty());
          assert(clientConns_.empty());
        }
      }
    }
  }

  void sendBackendString(int id, const string& msg)
  {
    assert(msg.size() <= kMaxPacketLen);
    Buffer buf;
    buf.append(msg);
    sendBackendPacket(id, &buf);
  }

  void onClientMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp)
  {
    if (!conn->getContext().empty())
    {
      int id = boost::any_cast<int>(conn->getContext());
      sendBackendBuffer(id, buf);
    }
    else
    {
      buf->retrieveAll();
      // FIXME: error handling
    }
  }

  void sendBackendBuffer(int id, Buffer* buf)
  {
    while (buf->readableBytes() > kMaxPacketLen)
    {
      Buffer packet;
      packet.append(buf->peek(), kMaxPacketLen);
      buf->retrieve(kMaxPacketLen);
      sendBackendPacket(id, &packet);
    }
    if (buf->readableBytes() > 0)
    {
      sendBackendPacket(id, buf);
    }
  }

  void sendBackendPacket(int id, Buffer* buf)
  {
    size_t len = buf->readableBytes();
    LOG_DEBUG << "sendBackendPacket " << len;
    assert(len <= kMaxPacketLen);
    uint8_t header[kHeaderLen] = {
      static_cast<uint8_t>(len),
      static_cast<uint8_t>(id & 0xFF),
      static_cast<uint8_t>((id & 0xFF00) >> 8)
    };
    buf->prepend(header, kHeaderLen);
    if (backendConn_)
    {
      backendConn_->send(buf);
    }
  }

  void onBackendConnection(const TcpConnectionPtr& conn)
  {
    LOG_TRACE << "Backend " << conn->localAddress().toIpPort() << " -> "
              << conn->peerAddress().toIpPort() << " is "
              << (conn->connected() ? "UP" : "DOWN");

    if (conn->connected())
    {
      backendConn_ = conn;
      assert(availIds_.empty());
      for (int i = 1; i <= kMaxConns; ++i)
      {
        availIds_.push(i);
      }
    }
    else
    {
      backendConn_.reset();
      for (std::map<int, TcpConnectionPtr>::iterator it = clientConns_.begin();
          it != clientConns_.end();
          ++it)
      {
        it->second->shutdown();
      }
      clientConns_.clear();
      while (!availIds_.empty())
      {
        availIds_.pop();
      }
    }
  }

  void onBackendMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp)
  {
    sendToClient(buf);
  }

  void sendToClient(Buffer* buf)
  {
    while (buf->readableBytes() > kHeaderLen)
    {
      int len = static_cast<uint8_t>(*buf->peek());
      if (buf->readableBytes() < len + kHeaderLen)
      {
        break;
      }
      else
      {
        int id = static_cast<uint8_t>(buf->peek()[1]);
        id |= (static_cast<uint8_t>(buf->peek()[2]) << 8);

        if (id != 0)
        {
          std::map<int, TcpConnectionPtr>::iterator it = clientConns_.find(id);
          if (it != clientConns_.end())
          {
            it->second->send(buf->peek() + kHeaderLen, len);
          }
        }
        else
        {
          string cmd(buf->peek() + kHeaderLen, len);
          LOG_INFO << "Backend cmd " << cmd;
          doCommand(cmd);
        }
        buf->retrieve(len + kHeaderLen);
      }
    }
  }

  void doCommand(const string& cmd)
  {
    static const string kDisconnectCmd = "DISCONNECT ";

    if (cmd.size() > kDisconnectCmd.size()
        && std::equal(kDisconnectCmd.begin(), kDisconnectCmd.end(), cmd.begin()))
    {
      int connId = atoi(&cmd[kDisconnectCmd.size()]);
      std::map<int, TcpConnectionPtr>::iterator it = clientConns_.find(connId);
      if (it != clientConns_.end())
      {
        it->second->shutdown();
      }
    }
  }

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
  if (argc > 1)
  {
    backendIp = argv[1];
  }
  InetAddress backendAddr(backendIp, kBackendPort);
  MultiplexServer server(&loop, listenAddr, backendAddr);

  server.start();

  loop.loop();
}

