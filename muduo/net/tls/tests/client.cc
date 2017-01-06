#include <muduo/net/EventLoop.h>
#include <muduo/net/tls/TlsClient.h>
#include <muduo/net/tls/TlsConfig.h>

using namespace muduo;
using namespace muduo::net;

int main(int argc, char* argv[])
{
  EventLoop loop;
  TlsConfig config;
  config.setCaFile(argv[1]);
  InetAddress serverAddr("127.0.0.1", 4433);
  TlsClient client(&loop, serverAddr, "Test Server Cert", &config);
  client.connect();
  loop.loop();
}
