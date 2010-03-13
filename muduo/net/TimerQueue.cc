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

struct timespec howMuchTimeFromNow(UtcTime when)
{
  int64_t microseconds = when.microSecondsSinceEpoch()
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

void resetTimerfd(int timerfd, UtcTime when)
{
  // wake up loop by timerfd_settime()
  struct itimerspec newValue;
  struct itimerspec oldValue;
  bzero(&newValue, sizeof newValue);
  bzero(&oldValue, sizeof oldValue);
  newValue.it_value = howMuchTimeFromNow(when);
  int ret = timerfd_settime(timerfd, 0, &newValue, &oldValue);
  if (ret)
  {
    perror("Error in timerfd_settime");
  }
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
  for (TimerList::iterator it = timers_.begin();
      it != timers_.end(); ++it)
  {
    delete *it;
  }
}

// FIXME replace linked-list operations with binary-heap.
void TimerQueue::timeout()
{
  loop_->assertInLoopThread();
  UtcTime now(UtcTime::now());
  uint64_t howmany;
  ssize_t n = ::read(timerfd_, &howmany, sizeof howmany);
  printf("TimerQueue::timeout() timeout %" PRIu64 " at %s\n", howmany, now.toString().c_str());
  if (n != sizeof howmany)
  {
    fprintf(stderr, "TimerQueue::timeout() reads %zd bytes instead of 8\n", n);
  }

  TimerList expired;

  // move out all expired timers
  {
    MutexLockGuard lock(mutex_);
    // shall never callback in critical section
    TimerList::iterator it = timers_.begin();
    while (it != timers_.end() && !(*it)->expiration().after(now))
    {
      ++it;
    }
    assert(it == timers_.end() || (*it)->expiration().after(now));
    expired.splice(expired.begin(), timers_, timers_.begin(), it);
  }

  // safe to callback outside critical section
  for (TimerList::iterator it = expired.begin();
      it != expired.end(); ++it)
  {
    (*it)->run();
  }

  UtcTime nextExpire;
  {
    MutexLockGuard lock(mutex_);
    // shall never callback in critical section

    for (TimerList::iterator it = expired.begin();
        it != expired.end(); ++it)
    {
      if ((*it)->repeat())
      {
        (*it)->restart(now);
        insertWithLockHold(*it);
      }
      else
      {
        delete *it;
      }
    }
    if (!timers_.empty())
    {
      nextExpire = timers_.front()->expiration();
    }
  }

  if (nextExpire.valid())
  {
    resetTimerfd(timerfd_, nextExpire);
  }
}

TimerId TimerQueue::schedule(const TimerCallback& cb, UtcTime when, double interval)
{
  Timer* timer = new Timer(cb, when, interval);
  
  bool earliestChanged = false;
  {
    MutexLockGuard lock(mutex_);
    // shall never callback in critical section
    earliestChanged = insertWithLockHold(timer);
  }
  if (earliestChanged)
  {
    resetTimerfd(timerfd_, when);
  }
  
  return TimerId(timer);
}

bool TimerQueue::insertWithLockHold(Timer* timer)
{
  bool earliestChanged = false;
  UtcTime when = timer->expiration();
  TimerList::iterator it = timers_.begin();
  if (it == timers_.end() || (*it)->expiration().after(when))
  {
    timers_.push_front(timer);
    earliestChanged = true;
  }
  else
  {
    while (it != timers_.end() && (*it)->expiration().before(when))
    {
      ++it;
    }
    timers_.insert(it, timer);
  }
  return earliestChanged;
}

