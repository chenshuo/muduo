#include <muduo/net/http/HttpServer.h>
#include <muduo/net/EventLoop.h>

using namespace muduo::net;

int main()
{
  EventLoop loop;
  HttpServer server(&loop, InetAddress(8000), "dummy");
  server.start();
  loop.loop();
}
