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
    : context_(TlsContext::kClient, config),
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
      // FIXME
      // TlsConnectionPtr tls(new TlsConnection(conn, &context_));
      // conn->setContext(tls);
    }
    else
    {
      conn->setContext(boost::any());
    }
  }

  TlsContext context_;
  TcpClient client_;
};

}  // namespace net
}  // namespace muduo
