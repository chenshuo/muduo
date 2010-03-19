// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include <muduo/net/Poller.h>
#include <muduo/net/EventLoop.h>

using namespace muduo;
using namespace muduo::net;

Poller::Poller(EventLoop* loop)
  : loop_(loop)
{
}

Poller::~Poller()
{
}

void Poller::assertInLoopThread()
{
  loop_->assertInLoopThread();
}

