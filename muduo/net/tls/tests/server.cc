#include <muduo/net/EventLoop.h>
#include <muduo/net/tls/TlsConfig.h>
#include <muduo/net/tls/TlsServer.h>

using namespace muduo;
using namespace muduo::net;

int main(int argc, char* argv[])
{
  EventLoop loop;
  TlsConfig config;
  config.setCertFile(argv[1]);
  config.setKeyFile(argv[2]);

  InetAddress listenAddr(4433);
  TlsServer server(&loop, listenAddr, "SSL Server", &config);
  server.start();
  loop.loop();
}

