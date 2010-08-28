// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include <muduo/net/TimerQueue.h>

#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/Timer.h>
#include <muduo/net/TimerId.h>

#include <boost/bind.hpp>

#include <sys/timerfd.h>

using namespace muduo;
using namespace muduo::net;

namespace
{

int createTimerfd()
{
  int timerfd = ::timerfd_create(CLOCK_MONOTONIC,
                                 TFD_NONBLOCK | TFD_CLOEXEC);
  if (timerfd < 0)
  {
    LOG_SYSFATAL << "Failed in timerfd_create";
  }
  return timerfd;
}

struct timespec howMuchTimeFromNow(Timestamp when)
{
  int64_t microseconds = when.microSecondsSinceEpoch()
                         - Timestamp::now().microSecondsSinceEpoch();
  if (microseconds < 100)
  {
    microseconds = 100;
  }
  struct timespec ts;
  ts.tv_sec = static_cast<time_t>(
      microseconds / Timestamp::kMicroSecondsPerSecond);
  ts.tv_nsec = static_cast<long>(
      (microseconds % Timestamp::kMicroSecondsPerSecond) * 1000);
  return ts;
}

void resetTimerfd(int timerfd, Timestamp when)
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
    LOG_SYSERR << "timerfd_settime()";
  }
}

}

TimerQueue::TimerQueue(EventLoop* loop)
  : loop_(loop),
    timerfd_(createTimerfd()),
    timerfdChannel_(loop, timerfd_),
    timers_()
{
  timerfdChannel_.setReadCallback(
      boost::bind(&TimerQueue::handleRead, this));
  // we are always reading the timerfd, we disarm it with timerfd_settime.
  timerfdChannel_.enableReading();
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
void TimerQueue::handleRead()
{
  loop_->assertInLoopThread();
  Timestamp now(Timestamp::now());
  uint64_t howmany;
  ssize_t n = ::read(timerfd_, &howmany, sizeof howmany);
  LOG_TRACE << "TimerQueue::handleRead() " << howmany << " at " << now.toString();
  if (n != sizeof howmany)
  {
    LOG_ERROR << "TimerQueue::handleRead() reads " << n << " bytes instead of 8";
  }

  TimerList expired;

  // move out all expired timers
  {
    MutexLockGuard lock(mutex_);
    // shall never callback in critical section
    TimerList::iterator it = timers_.begin();
    while (it != timers_.end() && (*it)->expiration() <= now)
    {
      ++it;
    }
    assert(it == timers_.end() || (*it)->expiration() > now);
    expired.splice(expired.begin(), timers_, timers_.begin(), it);
  }

  // safe to callback outside critical section
  for (TimerList::iterator it = expired.begin();
      it != expired.end(); ++it)
  {
    (*it)->run();
  }

  Timestamp nextExpire;
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
        // FIXME move to a free list
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

TimerId TimerQueue::schedule(const TimerCallback& cb,
                             Timestamp when,
                             double interval)
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
  Timestamp when = timer->expiration();
  TimerList::iterator it = timers_.begin();
  if (it == timers_.end() || (*it)->expiration() > when)
  {
    timers_.push_front(timer);
    earliestChanged = true;
  }
  else
  {
    while (it != timers_.end() && (*it)->expiration() < when)
    {
      ++it;
    }
    timers_.insert(it, timer);
  }
  return earliestChanged;
}

