#include <examples/cdns/Resolver.h>
#include <muduo/net/EventLoop.h>
#include <stdio.h>

using namespace muduo::net;
using namespace cdns;

EventLoop* g_loop;
int count = 0;

void quit()
{
  g_loop->quit();
}

void resolveCallback(const InetAddress& addr)
{
  printf("resolveCallback %s\n", addr.toHostPort().c_str());
  if (++count == 3)
    quit();
}

int main(int argc, char* argv[])
{
  EventLoop loop;
  loop.runAfter(10, quit);
  g_loop = &loop;
  Resolver resolver(&loop, Resolver::kDNSonly);
  resolver.resolve("www.chenshuo.com", resolveCallback);
  resolver.resolve("www.example.com", resolveCallback);
  resolver.resolve("www.google.com", resolveCallback);
  loop.loop();
}
