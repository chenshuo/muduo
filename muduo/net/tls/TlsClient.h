#pragma once

#include <muduo/net/TcpClient.h>
#include <muduo/net/tls/TlsConnection.h>
#include <muduo/net/tls/TlsContext.h>

#include <boost/bind.hpp>

namespace muduo {
namespace net {

class TlsClient : boost::noncopyable
{
 public:
  TlsClient(EventLoop* loop,
            const InetAddress& serverAddr,
            const string& serverName,
            TlsConfig* config)
    : config_(config),
      serverName_(serverName),
      client_(loop, serverAddr, serverName)
  {
    client_.setConnectionCallback(boost::bind(&TlsClient::onConnection, this, _1));
  }

  void connect()
  {
    client_.connect();
  }

 private:
  void onConnection(const TcpConnectionPtr& conn)
  {
    if (conn->connected())
    {
      TlsConnectionPtr tls(new TlsConnection(conn, config_, serverName_));
      conn->setContext(tls);
    }
    else
    {
      conn->setContext(boost::any());
    }
  }

  TlsConfig* config_;  // not owned
  string serverName_;
  TcpClient client_;
};

}  // namespace net
}  // namespace muduo
