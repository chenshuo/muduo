#pragma once

#include <muduo/base/Logging.h>
#include <muduo/base/StringPiece.h>
#include <boost/noncopyable.hpp>
#include <tls.h>

namespace muduo {
namespace net {

class TlsConfig : boost::noncopyable
{
 public:
  TlsConfig()
    : config_(CHECK_NOTNULL(tls_config_new()))
  {
    if (initialized <= 0)
    {
      LOG_FATAL;
    }
  }

  ~TlsConfig()
  {
    tls_config_free(config_);
  }

  void setCaFile(StringArg caFile)
  {
    Check(tls_config_set_ca_file(config_, caFile.c_str()));
  }

  void setCertFile(StringArg certFile)
  {
    Check(tls_config_set_cert_file(config_, certFile.c_str()));
  }

  void setKeyFile(StringArg keyFile)
  {
    Check(tls_config_set_key_file(config_, keyFile.c_str()));
  }

  struct tls_config* get() { return config_; }

 private:
  void Check(int ret)
  {
    if (ret != 0)
    {
      LOG_FATAL << tls_config_error(config_);
    }
  }

  struct tls_config* config_;

  static int initialized;
};

}  // namespace net
}  // namespace muduo
