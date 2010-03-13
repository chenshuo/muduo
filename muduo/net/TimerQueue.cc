#include <muduo/net/TimerQueue.h>

#include <muduo/net/EventLoop.h>
#include <muduo/net/Timer.h>
#include <muduo/net/TimerId.h>

#include <boost/bind.hpp>

#include <stdio.h>
#include <sys/timerfd.h>

using namespace muduo;
using namespace muduo::net;

namespace
{
int createTimerfd()
{
  int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
  if (timerfd < 0)
  {
    perror("Failed in timerfd_create");
    abort();
  }
  return timerfd;
}
}

TimerQueue::TimerQueue(EventLoop* loop)
  : loop_(loop),
    timerfd_(createTimerfd()),
    timerfdChannel_(loop, timerfd_),
    timers_()
{
  timerfdChannel_.setReadCallback(boost::bind(&TimerQueue::timeout, this));
}

TimerQueue::~TimerQueue()
{
  ::close(timerfd_);
  // do not remove channel, since we're in EventLoop::dtor();
  /*
  for (Alarms::iterator it(alarms_.begin());
      it != alarms_.end();
      ++it)
  {
    delete it->second;
  }
  */
}

void TimerQueue::timeout()
{
  /*
  while (!alarms_.empty())
  {
    Alarms::iterator head(alarms_.begin());
    if (head->first.after(now))
    {
      break;
    }
    else
    {
      Alarm* alarm = head->second;
      alarm->run();
      UtcTime next = alarm->getNextTime(now);
      if (next.valid())
      {
        alarms_.insert(std::make_pair(next, alarm));
      }
      else
      {
        delete alarm;
      }
      alarms_.erase(head);
    }
  }
  return alarms_.empty() ? UtcTime() : alarms_.begin()->first;
  */
  timerfdChannel_.set_events(0);
  loop_->updateChannel(&timerfdChannel_);
}

TimerId TimerQueue::schedule(const TimerCallback& cb, UtcTime at, double interval)
{
  Timer* timer = new Timer(cb, at, interval);
  timers_.push_front(timer);
  timerfdChannel_.set_events(Channel::kReadEvent);
  loop_->updateChannel(&timerfdChannel_);
  return TimerId(timer);
}

