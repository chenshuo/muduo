#include <muduo/net/EventLoop.h>

#include <stdio.h>
#include <unistd.h>

using namespace muduo;
using namespace muduo::net;

void print()
{
  printf("%s\n", UtcTime::now().toString().c_str());
}

int main()
{
  printf("pid = %d\n", getpid());
  EventLoop loop;

  print();
  loop.runAfter(1.0, print);

  loop.loop();
}
