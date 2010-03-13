#include <muduo/net/Poller.h>
#include <muduo/net/PollPoller.h>
#include <muduo/net/EPollPoller.h>

using namespace muduo::net;

Poller* Poller::newDefaultPoller()
{
  return new PollPoller;
}
