#include <muduo/net/inspect/Inspector.h>
#include <muduo/net/EventLoop.h>

using namespace muduo;
using namespace muduo::net;

int main()
{
  EventLoop loop;
  Inspector ins(&loop, InetAddress(12345), "test");
  loop.loop();
}

