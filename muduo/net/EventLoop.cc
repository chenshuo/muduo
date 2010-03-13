#include <muduo/net/EventLoop.h>

#include <muduo/base/UtcTime.h>
#include <muduo/net/Channel.h>
//#include <net/internal/Log.h>
#include <muduo/net/Poller.h>
#include <muduo/net/TimerQueue.h>

using namespace muduo;
using namespace muduo::net;

EventLoop::EventLoop()
  : poller_(Poller::newDefaultPoller()),
    timerQueue_(new TimerQueue),
    quit_(false)
{
  init();
}

EventLoop::~EventLoop()
{
}

void EventLoop::loop()
{
  while (!quit_)
  {
    poller_->poll(1000);
  }
  /*
  while (!quit_)
  {
    UtcTime now(UtcTime::now());
    UtcTime next(timerQueue_->tick(now));
    int timeout = next.valid() ? static_cast<int>((timeDifference(next, now))*1000) : 1000;
    if (timeout <= 0)
    {
      timeout = 1;
    }

    NetLogInfo << "polling " << timeout << NetSend;
    poller_->poll(timeout);
  }
  */
}

void EventLoop::quit()
{
  quit_ = true;
}

void EventLoop::addChannel(Channel* channel)
{
  assert(channel->getLoop() == this);
  // channel->set_loop(this);
  // poller_->addChannel(channel);
}

void EventLoop::removeChannel(Channel* channel)
{
  assert(channel->getLoop() == this);
  // poller_->removeChannel(channel);
}

void EventLoop::init()
{
}

TimerId EventLoop::runAt(const UtcTime& time, const TimerCallback& cb)
{
  return timerQueue_->schedule(cb, time, 0.0);
}

TimerId EventLoop::runAfter(double delay, const TimerCallback& cb)
{
  UtcTime time(addTime(UtcTime::now(), delay));
  return runAt(time, cb);
}

TimerId EventLoop::runEvery(double interval, const TimerCallback& cb)
{
  UtcTime time(addTime(UtcTime::now(), interval));
  return timerQueue_->schedule(cb, time, interval);
}

