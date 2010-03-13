#include <muduo/net/Poller.cc>
#include <muduo/net/PollPoller.cc>

Poller* Poller::newDefaultPoller()
{
  return new PollPoller;
}
