#include <muduo/net/EventLoop.h>
#include <muduo/base/Thread.h>
#include <muduo/base/Logging.h>

#include <boost/bind.hpp>

#include <assert.h>
#include <stdio.h>
#include <unistd.h>

using namespace muduo;
using namespace muduo::net;

EventLoop* g_loop;

void quit()
{
  LOG_INFO << __func__ << "pid = " << getpid() << " tid = " << CurrentThread::tid();
  g_loop->quit();
}

int main()
{
  LOG_INFO << __func__ << "pid = " << getpid() << " tid = " << CurrentThread::tid();

  assert(EventLoop::getEventLoopOfCurrentThread() == NULL);
  EventLoop loop;
  g_loop = &loop;
  assert(EventLoop::getEventLoopOfCurrentThread() == &loop);
  loop.queueInLoop(&quit);

  loop.loop();
}
