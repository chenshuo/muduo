#pragma once

#include <muduo/net/TcpServer.h>
#include <muduo/net/tls/TlsConnection.h>
#include <muduo/net/tls/TlsContext.h>

#include <boost/bind.hpp>

namespace muduo {
namespace net {

class TlsServer : boost::noncopyable
{
 public:
  TlsServer(EventLoop* loop,
            const InetAddress& listenAddr,
            const string& name,
            TlsConfig* config)
    : context_(TlsContext::kServer, config),
      server_(loop, listenAddr, name)
  {
    server_.setConnectionCallback(boost::bind(&TlsServer::onConnection, this, _1));
  }

  void start()
  {
    server_.start();
  }

 private:
  void onConnection(const TcpConnectionPtr& conn)
  {
    if (conn->connected())
    {
      TlsConnectionPtr tls(new TlsConnection(conn, &context_));
      conn->setContext(tls);
    }
    else
    {
      conn->setContext(boost::any());
    }
  }

  TlsContext context_;
  TcpServer server_;
};

}  // namespace net
}  // namespace muduo
