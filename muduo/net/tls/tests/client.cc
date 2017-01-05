#include <muduo/net/EventLoop.h>
#include <muduo/net/tls/TlsClient.h>
#include <muduo/net/tls/TlsConfig.h>

using namespace muduo;
using namespace muduo::net;

int main(int argc, char* argv[])
{
  EventLoop loop;
  TlsConfig config;
  InetAddress serverAddr("127.0.0.1", 4433);
  TlsClient client(&loop, serverAddr, "chenshuo.com", &config);
  client.connect();
  loop.loop();
}
