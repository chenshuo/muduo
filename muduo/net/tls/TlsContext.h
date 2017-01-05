#pragma once

#include <muduo/net/tls/TlsConfig.h>

namespace muduo {
namespace net {

class TlsContext : boost::noncopyable
{
 public:
  enum Endpoint { kClient, kServer };

  TlsContext(Endpoint type, TlsConfig* config)
    : context_(type == kServer ? tls_server() : tls_client())
  {
    check(tls_configure(context_, config->get()));
  }

  TlsContext() : context_(NULL) {}

  ~TlsContext()
  {
    tls_free(context_);
  }

  void reset(struct tls* ctx) { context_ = ctx; }

  struct tls* get() { return context_; }

  // void accept(
 private:
  void check(int ret)
  {
    if (ret != 0)
    {
      LOG_FATAL << tls_error(context_);
    }
  }

  struct tls* context_;
};

}  // namespace net
}  // namespace muduo
