// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include <muduo/net/Poller.h>
#include <muduo/net/PollPoller.h>
#include <muduo/net/EPollPoller.h>

using namespace muduo::net;

Poller* Poller::newDefaultPoller(EventLoop* loop)
{
  return new PollPoller(loop);
}
