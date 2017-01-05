#pragma once

#include <muduo/net/tls/TlsContext.h>

#include <boost/bind.hpp>

namespace muduo {
namespace net {

class TlsConnection : boost::noncopyable
{
 public:
  TlsConnection(const TcpConnectionPtr& conn, TlsContext* context)
    : conn_(conn), state_(kHandshaking)
  {
    conn_->setMessageCallback(boost::bind(&TlsConnection::onMessage, this, _1, _2, _3));

    struct tls* conn_ctx = NULL;
    tls_accept_cbs(context->get(), &conn_ctx, net_read, net_write, this);  // FIXME
    context_.reset(conn_ctx);
    handshake();
  }

  // FIXME: shutdown
 private:
  void handshake()
  {
    int ret = tls_handshake(context_.get());
    if (ret == 0)
    {
      LOG_INFO << "OK";
      LOG_INFO << "Version " << tls_conn_version(context_.get());
      LOG_INFO << "Cipher " << tls_conn_cipher(context_.get());
      LOG_INFO << "Name " << tls_conn_servername(context_.get());
      state_ = kEstablished;
      tls_write(context_.get(), "Hello\n", 6);
    }
    else if (ret != TLS_WANT_POLLIN && ret != TLS_WANT_POLLOUT)
    {
      LOG_ERROR << "WRONG";
      conn_->shutdown();
      // FIXME
    }
    else
    {
      LOG_DEBUG << ((ret == TLS_WANT_POLLIN) ? "WANT_READ" : "WANT_WRITE");
    }
  }

  void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time)
  {
    LOG_INFO << "readable " << buf->readableBytes();
    if (state_ == kHandshaking)
    {
      handshake();
    }
    else
    {
      // FIXME: read
      
      char tbuf[1024];
      ssize_t nr = tls_read(context_.get(), tbuf, sizeof tbuf);
      LOG_INFO << "NR " << nr;
      nr = tls_read(context_.get(), tbuf, sizeof tbuf);
      LOG_INFO << "NR " << nr;
    }
  }

  static ssize_t net_read(struct tls *ctx, void *buf, size_t len, void *arg)
  {
    LOG_DEBUG << len;
    TlsConnection* self = static_cast<TlsConnection*>(arg);
    Buffer* in = self->conn_->inputBuffer();
    if (in->readableBytes() > 0)
    {
      size_t n = std::min(in->readableBytes(), len);
      memcpy(buf, in->peek(), n);
      in->retrieve(n);
      return n;
    }
    else
      return TLS_WANT_POLLIN;
  }

  static ssize_t net_write(struct tls *ctx, const void *buf, size_t len, void *arg)
  {
    LOG_DEBUG << len;
    TlsConnection* self = static_cast<TlsConnection*>(arg);
    self->conn_->send(buf, static_cast<int>(len));
    return len;
  }

  TcpConnectionPtr conn_;
  TlsContext context_;
  enum State { kHandshaking, kEstablished };
  State state_;
};
typedef boost::shared_ptr<TlsConnection> TlsConnectionPtr;

}  // namespace net
}  // namespace muduo
