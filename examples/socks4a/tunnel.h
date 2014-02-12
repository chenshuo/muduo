#ifndef MUDUO_EXAMPLES_SOCKS4A_TUNNEL_H
#define MUDUO_EXAMPLES_SOCKS4A_TUNNEL_H

#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/TcpClient.h>
#include <muduo/net/TcpServer.h>

class Tunnel : public std::enable_shared_from_this<Tunnel>,
               boost::noncopyable
{
 public:
  Tunnel(muduo::net::EventLoop* loop,
         const muduo::net::InetAddress& serverAddr,
         const muduo::net::TcpConnectionPtr& serverConn)
    : client_(loop, serverAddr, serverConn->name()),
      serverConn_(serverConn)
  {
    LOG_INFO << "Tunnel " << serverConn->peerAddress().toIpPort()
             << " <-> " << serverAddr.toIpPort();
  }

  ~Tunnel()
  {
    LOG_INFO << "~Tunnel";
  }

  void setup()
  {
    using std::placeholders::_1;
    using std::placeholders::_2;
    using std::placeholders::_3;

    client_.setConnectionCallback(
        std::bind(&Tunnel::onClientConnection, shared_from_this(), _1));
    client_.setMessageCallback(
        std::bind(&Tunnel::onClientMessage, shared_from_this(), _1, _2, _3));
    serverConn_->setHighWaterMarkCallback(
        std::bind(&Tunnel::onHighWaterMarkWeak, std::weak_ptr<Tunnel>(shared_from_this()), _1, _2),
        10*1024*1024);
  }

  void teardown()
  {
    client_.setConnectionCallback(muduo::net::defaultConnectionCallback);
    client_.setMessageCallback(muduo::net::defaultMessageCallback);
    if (serverConn_)
    {
      serverConn_->setContext(boost::any());
      serverConn_->shutdown();
    }
  }

  void connect()
  {
    client_.connect();
  }

  void disconnect()
  {
    client_.disconnect();
    // serverConn_.reset();
  }

  void onClientConnection(const muduo::net::TcpConnectionPtr& conn)
  {
    using std::placeholders::_1;
    using std::placeholders::_2;

    LOG_DEBUG << (conn->connected() ? "UP" : "DOWN");
    if (conn->connected())
    {
      conn->setTcpNoDelay(true);
      conn->setHighWaterMarkCallback(
          std::bind(&Tunnel::onHighWaterMarkWeak, std::weak_ptr<Tunnel>(shared_from_this()), _1, _2),
          10*1024*1024);
      serverConn_->setContext(conn);
      if (serverConn_->inputBuffer()->readableBytes() > 0)
      {
        conn->send(serverConn_->inputBuffer());
      }
    }
    else
    {
      teardown();
    }
  }

  void onClientMessage(const muduo::net::TcpConnectionPtr& conn,
                       muduo::net::Buffer* buf,
                       muduo::Timestamp)
  {
    LOG_DEBUG << conn->name() << " " << buf->readableBytes();
    if (serverConn_)
    {
      serverConn_->send(buf);
    }
    else
    {
      buf->retrieveAll();
      abort();
    }
  }

  void onHighWaterMark(const muduo::net::TcpConnectionPtr& conn,
                       size_t bytesToSent)
  {
    LOG_INFO << "onHighWaterMark " << conn->name()
             << " bytes " << bytesToSent;
    disconnect();
  }

  static void onHighWaterMarkWeak(const std::weak_ptr<Tunnel>& wkTunnel,
                                  const muduo::net::TcpConnectionPtr& conn,
                                  size_t bytesToSent)
  {
    std::shared_ptr<Tunnel> tunnel = wkTunnel.lock();
    if (tunnel)
    {
      tunnel->onHighWaterMark(conn, bytesToSent);
    }
  }

 private:
  muduo::net::TcpClient client_;
  muduo::net::TcpConnectionPtr serverConn_;
};
typedef std::shared_ptr<Tunnel> TunnelPtr;

#endif
