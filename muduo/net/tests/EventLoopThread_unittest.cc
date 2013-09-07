#include <muduo/net/EventLoopThread.h>
#include <muduo/net/EventLoop.h>
#include <muduo/base/Thread.h>
#include <muduo/base/CountDownLatch.h>

#include <boost/bind.hpp>

#include <stdio.h>

using namespace muduo;
using namespace muduo::net;

CountDownLatch g_latch(2);

void print(EventLoop* p = NULL)
{
  printf("print: pid = %d, tid = %d, loop = %p\n",
         getpid(), CurrentThread::tid(), p);
  g_latch.countDown();
}

int main()
{
  print();

  EventLoopThread thr1;

  EventLoopThread thr2;
  EventLoop* loop = thr2.startLoop();
  loop->runInLoop(boost::bind(print, loop));
  g_latch.wait();
}

