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
#include <inttypes.h>

using namespace muduo;
using namespace muduo::net;

bool TimerQueue::lesscomp(TimerPtr lhs, TimerPtr rhs)
{
  if (lhs->expiration() == rhs->expiration())
  {
    return lhs.get() < rhs.get();
  }
  else
  {
    return lhs->expiration() < rhs->expiration();
  }
}

TimerQueue::TimerQueue(EventLoop *loop)
    : loop_(loop), timers_(&TimerQueue::lesscomp), timersCancle_(&TimerQueue::lesscomp)
{
}

TimerQueue::~TimerQueue()
{
}

TimerId TimerQueue::addTimer(TimerCallback cb, Timestamp when, double interval)
{
  TimerPtr timer(new Timer(std::move(cb), when, interval));
  loop_->runInLoop(std::bind(&TimerQueue::addTimerInLoop, this, timer));
  return TimerId(timer);
}

void TimerQueue::cancel(TimerId timerId)
{
  loop_->runInLoop(std::bind(&TimerQueue::cancelInLoop, this, timerId));
}

void TimerQueue::handleTimerEvent(Timestamp stamp)
{
  loop_->assertInLoopThread();

  std::vector<TimerPtr> expired;
  getExpired(stamp, expired);

  for (auto &it : expired)
  {
    it->run();
  }

  reset(expired, stamp);
}

int TimerQueue::getNextTimeout(Timestamp stamp)
{
  if (timers_.empty())
  {
    return 10 * 1000;
  }
  else
  {
    TimerList::iterator it = timers_.begin();
    int64_t t = (*it)->expiration() - stamp;
    return t < 0 ? 0 : static_cast<int>(t);
  }
}

void TimerQueue::addTimerInLoop(TimerPtr timer)
{
  loop_->assertInLoopThread();
  std::pair<TimerList::iterator, bool> result = timers_.insert(timer);
  assert(result.second);
  (void)result;
}

void TimerQueue::cancelInLoop(TimerId timerId)
{
  loop_->assertInLoopThread();
  TimerPtr target = timerId.timer_.lock();
  if (target)
  {
    auto it_find = timers_.find(target);
    if (it_find == timers_.end())
    {
      timersCancle_.insert(target);
    }
    else
    {
      timers_.erase(it_find);
    }
  }
}

void TimerQueue::getExpired(Timestamp now, std::vector<TimerPtr> &expired)
{
  auto it = timers_.begin();
  for (; it != timers_.end(); it++)
  {
    if (now < (*it)->expiration())
    {
      break;
    }
    else
    {
      if (timersCancle_.find(*it) == timersCancle_.end())
      {
        expired.push_back(*it);
      }
    }
  }
  timers_.erase(timers_.begin(), it);
}

void TimerQueue::reset(std::vector<TimerPtr> &expired, Timestamp now)
{
  for (auto &it : timersCancle_)
  {
    auto it_find = timers_.find(it);
    if (it_find != timers_.end())
    {
      timers_.erase(it_find);
    }
  }

  for (auto &it : expired)
  {
    if (it->repeat() && timersCancle_.find(it) == timersCancle_.end())
    {
      it->restart(now);
      timers_.insert(it);
    }
  }

  timersCancle_.clear();
}
