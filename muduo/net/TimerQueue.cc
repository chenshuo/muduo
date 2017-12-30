// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#include <muduo/net/TimerQueue.h>

#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/TimerId.h>
#include <muduo/net/Timer.h>

#include <sys/timerfd.h>
#include <unistd.h>


namespace muduo
{
namespace net
{

namespace detail
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
  int64_t microseconds = when.microSecondsSinceEpoch() - Timestamp::now().microSecondsSinceEpoch();
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

void readTimerfd(int timerfd, Timestamp now)
{
  uint64_t howmany;
  ssize_t n = ::read(timerfd, &howmany, sizeof howmany);
  LOG_TRACE << "TimerQueue::handleRead() " << howmany << " at " << now.toString();
  if (n != sizeof howmany)
  {
    LOG_ERROR << "TimerQueue::handleRead() reads " << n << " bytes instead of 8";
  }
}

void resetTimerfd(int timerfd, Timestamp expiration)
{
  // wake up loop by timerfd_settime()
  struct itimerspec newValue;
  struct itimerspec oldValue;
  bzero(&newValue, sizeof newValue);
  bzero(&oldValue, sizeof oldValue);
  newValue.it_value = howMuchTimeFromNow(expiration);
  int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
  if (ret)
  {
    LOG_SYSERR << "timerfd_settime()";
  }
}
}
}
}

using namespace muduo;
using namespace muduo::net;
using namespace muduo::net::detail;

bool TimerQueue::lesscomp(TimerPtr lhs, TimerPtr rhs)
{
  if(lhs->expiration() == rhs->expiration())
  {
     return lhs.get() < rhs.get();
  }
  else
  {
    return lhs->expiration() < rhs->expiration();
  }
}

TimerQueue::TimerQueue(EventLoop *loop)
    : loop_(loop)
    , timerfd_(createTimerfd())
    , timerfdChannel_(loop, timerfd_)
    , timers_(&TimerQueue::lesscomp)
    , timersCancle_(&TimerQueue::lesscomp)
{
  timerfdChannel_.setReadCallback(
      std::bind(&TimerQueue::handleRead, this));
  // we are always reading the timerfd, we disarm it with timerfd_settime.
  timerfdChannel_.enableReading();
}

TimerQueue::~TimerQueue()
{
  timerfdChannel_.disableAll();
  timerfdChannel_.remove();
  ::close(timerfd_);
}

TimerId TimerQueue::addTimer(TimerCallback cb,
                             Timestamp when,
                             double interval)
{
  TimerPtr timer(new Timer(std::move(cb), when, interval));

  loop_->runInLoop(
      std::bind(&TimerQueue::addTimerInLoop, this, timer));
  return TimerId(timer);
}

void TimerQueue::cancel(TimerId timerId)
{
  loop_->runInLoop(
      std::bind(&TimerQueue::cancelInLoop, this, timerId));
}

void TimerQueue::addTimerInLoop(TimerPtr timer)
{
  loop_->assertInLoopThread();
  bool earliestChanged = insert(timer);

  if (earliestChanged)
  {
    resetTimerfd(timerfd_, timer->expiration());
  }
}

void TimerQueue::cancelInLoop(TimerId timerId)
{
  loop_->assertInLoopThread();
  TimerPtr target = timerId.timer_.lock();
  if (target)
  {
    auto it_find = timers_.find(target);
    if(it_find == timers_.end())
    {
      timersCancle_.insert(target);
    }
    else
    {
      timers_.erase(it_find);
    }
  }
}

void TimerQueue::handleRead()
{
  loop_->assertInLoopThread();
  Timestamp now(Timestamp::now());
  readTimerfd(timerfd_, now);

  std::vector<TimerPtr> expired;
  getExpired(now, expired);

  // safe to callback outside critical section
  for (auto &it : expired)
  {
    if(timersCancle_.find(it) == timersCancle_.end())
    {
      it->run();
    }
  }

  reset(expired, now);
}

void TimerQueue::getExpired(Timestamp now, std::vector<TimerPtr> &expired)
{
  TimerPtr timer(new Timer(0, now, 0));
  TimerList::iterator end = timers_.lower_bound(timer);
  assert(end == timers_.end() || now < (*end)->expiration());
  std::copy(timers_.begin(), end, back_inserter(expired));
  timers_.erase(timers_.begin(), end);
}

void TimerQueue::reset(std::vector<TimerPtr> &expired, Timestamp now)
{
  for (auto &it : timersCancle_)
  {
    auto it_find = timers_.find(it);
    if(it_find != timers_.end())
    {
      timers_.erase(it_find);
    }
  }

  for (auto &it : expired)
  {
    if (it->repeat() && timersCancle_.find(it) == timersCancle_.end())
    {
      it->restart(now);
      insert(it);
    }
  }
  timersCancle_.clear();

  Timestamp nextExpire;
  if (!timers_.empty())
  {
    nextExpire = timers_.begin()->get()->expiration();
  }

  if (nextExpire.valid())
  {
    resetTimerfd(timerfd_, nextExpire);
  }
}

bool TimerQueue::insert(TimerPtr &timer)
{
  loop_->assertInLoopThread();
  bool earliestChanged = false;
  Timestamp when = timer->expiration();
  auto it = timers_.begin();
  if (it == timers_.end() || when < (*it)->expiration())
  {
    earliestChanged = true;
  }
  {
    std::pair<TimerList::iterator, bool> result = timers_.insert(timer);
    assert(result.second);
    (void)result;
  }
  return earliestChanged;
}
