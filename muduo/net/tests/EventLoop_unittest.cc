#include <muduo/net/EventLoop.h>

#include <unistd.h>

using namespace muduo::net;

int main()
{
  EventLoop theLoop;
  theLoop.loop();
}
