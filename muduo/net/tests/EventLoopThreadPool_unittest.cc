#include <muduo/net/EventLoopThreadPool.h>
#include <muduo/net/EventLoop.h>
#include <muduo/base/Thread.h>

#include <boost/bind.hpp>

#include <stdio.h>

using namespace muduo;
using namespace muduo::net;

void print()
{
  printf("main(): pid = %d, tid = %d\n", getpid(), CurrentThread::tid());
}

int main()
{
  print();

  EventLoop loop;
  loop.runAfter(11, boost::bind(&EventLoop::quit, &loop));

  {
    printf("Single thread:\n");
    EventLoopThreadPool model(&loop);
    model.setThreadNum(0);
    model.start();
    assert(model.getNextLoop() == &loop);
    assert(model.getNextLoop() == &loop);
    assert(model.getNextLoop() == &loop);
  }

  {
    printf("Another thread:\n");
    EventLoopThreadPool model(&loop);
    model.setThreadNum(1);
    model.start();
    EventLoop* nextLoop = model.getNextLoop();
    (void)nextLoop;
    assert(nextLoop != &loop);
    assert(nextLoop == model.getNextLoop());
    assert(nextLoop == model.getNextLoop());
  }

  {
    printf("Three threads:\n");
    EventLoopThreadPool model(&loop);
    model.setThreadNum(3);
    model.start();
    EventLoop* nextLoop = model.getNextLoop();
    (void)nextLoop;
    assert(nextLoop != &loop);
    assert(nextLoop != model.getNextLoop());
    assert(nextLoop != model.getNextLoop());
    assert(nextLoop == model.getNextLoop());
  }

  loop.loop();
}

