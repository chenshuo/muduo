#include <muduo/net/EventLoop.h>
#include <muduo/base/Thread.h>

#include <boost/bind.hpp>

#include <stdio.h>
#include <unistd.h>

using namespace muduo;
using namespace muduo::net;

EventLoop* g_loop;

void threadFunc()
{
  EventLoop loop;
  loop.loop();
}

int main()
{
  printf("pid = %d, tid = %d\n", getpid(), CurrentThread::tid());

  EventLoop loop;

  Thread thread(threadFunc);
  thread.start();

  loop.loop();
}
