
#include <muduo/net/tls/TlsConfig.h>

int muduo::net::TlsConfig::initialized = tls_init() + 1;
