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

typedef std::shared_ptr<TcpClient> TcpClientPtr;

// const int kMaxConns = 1;
const size_t kMaxPacketLen = 255;
const size_t kHeaderLen = 3;

const uint16_t kListenPort = 9999;
const char* socksIp = "127.0.0.1";
const uint16_t kSocksPort = 7777;

struct Entry
{
  int connId;
  TcpClientPtr client;
  TcpConnectionPtr connection;
  Buffer pending;
};

class DemuxServer : noncopyable
{
 public:
  DemuxServer(EventLoop* loop, const InetAddress& listenAddr, const InetAddress& socksAddr)
    : loop_(loop),
      server_(loop, listenAddr, "DemuxServer"),
      socksAddr_(socksAddr)
  {
    server_.setConnectionCallback(
        std::bind(&DemuxServer::onServerConnection, this, _1));
    server_.setMessageCallback(
        std::bind(&DemuxServer::onServerMessage, this, _1, _2, _3));
  }

  void start()
  {
    server_.start();
  }

  void onServerConnection(const TcpConnectionPtr& conn)
  {
    if (conn->connected())
    {
      if (serverConn_)
      {
        conn->shutdown();
      }
      else
      {
        serverConn_ = conn;
        LOG_INFO << "onServerConnection set serverConn_";
      }
    }
    else
    {
      if (serverConn_ == conn)
      {
        serverConn_.reset();
        socksConns_.clear();

        LOG_INFO << "onServerConnection reset serverConn_";
      }
    }
  }

  void onServerMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp)
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
        int connId = static_cast<uint8_t>(buf->peek()[1]);
        connId |= (static_cast<uint8_t>(buf->peek()[2]) << 8);

        if (connId != 0)
        {
          assert(socksConns_.find(connId) != socksConns_.end());
          TcpConnectionPtr& socksConn = socksConns_[connId].connection;
          if (socksConn)
          {
            assert(socksConns_[connId].pending.readableBytes() == 0);
            socksConn->send(buf->peek() + kHeaderLen, len);
          }
          else
          {
            socksConns_[connId].pending.append(buf->peek() + kHeaderLen, len);
          }
        }
        else
        {
          string cmd(buf->peek() + kHeaderLen, len);
          doCommand(cmd);
        }
        buf->retrieve(len + kHeaderLen);
      }
    }
  }

  void doCommand(const string& cmd)
  {
    static const string kConn = "CONN ";

    int connId = atoi(&cmd[kConn.size()]);
    bool isUp = cmd.find(" IS UP") != string::npos;
    LOG_INFO << "doCommand " << connId << " " << isUp;
    if (isUp)
    {
      assert(socksConns_.find(connId) == socksConns_.end());
      char connName[256];
      snprintf(connName, sizeof connName, "SocksClient %d", connId);
      Entry entry;
      entry.connId = connId;
      entry.client.reset(new TcpClient(loop_, socksAddr_, connName));
      entry.client->setConnectionCallback(
          std::bind(&DemuxServer::onSocksConnection, this, connId, _1));
      entry.client->setMessageCallback(
          std::bind(&DemuxServer::onSocksMessage, this, connId, _1, _2, _3));
      // FIXME: setWriteCompleteCallback
      socksConns_[connId] = entry;
      entry.client->connect();
    }
    else
    {
      assert(socksConns_.find(connId) != socksConns_.end());
      TcpConnectionPtr& socksConn = socksConns_[connId].connection;
      if (socksConn)
      {
        socksConn->shutdown();
      }
      else
      {
        socksConns_.erase(connId);
      }
    }
  }

  void onSocksConnection(int connId, const TcpConnectionPtr& conn)
  {
    assert(socksConns_.find(connId) != socksConns_.end());
    if (conn->connected())
    {
      socksConns_[connId].connection = conn;
      Buffer& pendingData = socksConns_[connId].pending;
      if (pendingData.readableBytes() > 0)
      {
        conn->send(&pendingData);
      }
    }
    else
    {
      if (serverConn_)
      {
        char buf[256];
        int len = snprintf(buf, sizeof(buf), "DISCONNECT %d\r\n", connId);
        Buffer buffer;
        buffer.append(buf, len);
        sendServerPacket(0, &buffer);
      }
      else
      {
        socksConns_.erase(connId);
      }
    }
  }

  void onSocksMessage(int connId, const TcpConnectionPtr& conn, Buffer* buf, Timestamp)
  {
    assert(socksConns_.find(connId) != socksConns_.end());
    while (buf->readableBytes() > kMaxPacketLen)
    {
      Buffer packet;
      packet.append(buf->peek(), kMaxPacketLen);
      buf->retrieve(kMaxPacketLen);
      sendServerPacket(connId, &packet);
    }
    if (buf->readableBytes() > 0)
    {
      sendServerPacket(connId, buf);
    }
  }

  void sendServerPacket(int connId, Buffer* buf)
  {
    size_t len = buf->readableBytes();
    LOG_DEBUG << len;
    assert(len <= kMaxPacketLen);
    uint8_t header[kHeaderLen] = {
      static_cast<uint8_t>(len),
      static_cast<uint8_t>(connId & 0xFF),
      static_cast<uint8_t>((connId & 0xFF00) >> 8)
    };
    buf->prepend(header, kHeaderLen);
    if (serverConn_)
    {
      serverConn_->send(buf);
    }
  }

  EventLoop* loop_;
  TcpServer server_;
  TcpConnectionPtr serverConn_;
  const InetAddress socksAddr_;
  std::map<int, Entry> socksConns_;
};

int main(int argc, char* argv[])
{
  LOG_INFO << "pid = " << getpid();
  EventLoop loop;
  InetAddress listenAddr(kListenPort);
  if (argc > 1)
  {
    socksIp = argv[1];
  }
  InetAddress socksAddr(socksIp, kSocksPort);
  DemuxServer server(&loop, listenAddr, socksAddr);

  server.start();

  loop.loop();
}

