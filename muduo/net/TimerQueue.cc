#include <muduo/net/TimerQueue.h>

#include <muduo/net/EventLoop.h>
#include <muduo/net/Timer.h>
#include <muduo/net/TimerId.h>

#include <boost/bind.hpp>

#define __STDC_FORMAT_MACROS
#include <inttypes.h> // FIXME remove
#undef __STDC_FORMAT_MACROS
#include <stdio.h> // FIXME perror
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

struct timespec howMuchTimeFromNow(UtcTime then)
{
  int64_t microseconds = then.microSecondsSinceEpoch()
                         - UtcTime::now().microSecondsSinceEpoch();
  if (microseconds < 100)
  {
    microseconds = 100;
  }
  struct timespec ts;
  ts.tv_sec = static_cast<time_t>(microseconds / UtcTime::kMicroSecondsPerSecond);
  ts.tv_nsec = static_cast<long>((microseconds % UtcTime::kMicroSecondsPerSecond) * 1000);
  return ts;
}
}

TimerQueue::TimerQueue(EventLoop* loop)
  : loop_(loop),
    timerfd_(createTimerfd()),
    timerfdChannel_(loop, timerfd_),
    timers_()
{
  timerfdChannel_.setReadCallback(boost::bind(&TimerQueue::timeout, this));
  // we are always reading the timerfd, we disarm it with timerfd_settime.
  timerfdChannel_.set_events(Channel::kReadEvent);
  loop_->updateChannel(&timerfdChannel_);
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
  uint64_t howmany;
  ssize_t n = ::read(timerfd_, &howmany, sizeof howmany);
  printf("timeout %" PRIu64 "\n", howmany);
  if (n != sizeof howmany)
  {
    fprintf(stderr, "TimerQueue::timeout() reads %zd bytes instead of 8\n", n);
  }
}

TimerId TimerQueue::schedule(const TimerCallback& cb, UtcTime at, double interval)
{
  Timer* timer = new Timer(cb, at, interval);
  
  bool earliestChanged = false;
  {
    MutexLockGuard lock(mutex_);
    TimerList::iterator it = timers_.begin();
    if (it == timers_.end() || (*it)->expiration().after(at))
    {
      timers_.push_front(timer);
      earliestChanged = true;
    }
    else
    {
      while (it != timers_.end() && (*it)->expiration().before(at))
      {
        ++it;
      }
      timers_.insert(it, timer);
    }
  }

  if (earliestChanged)
  {
    // wake up loop by timerfd_settime()
    struct itimerspec newValue;
    struct itimerspec oldValue;
    bzero(&newValue, sizeof newValue);
    bzero(&oldValue, sizeof oldValue);
    newValue.it_value = howMuchTimeFromNow(at);
    int ret = timerfd_settime(timerfd_, 0, &newValue, &oldValue);
    if (ret)
    {
      perror("Error in timerfd_settime");
    }
  }
  
  return TimerId(timer);
}

