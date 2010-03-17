// Muduo - A lightwight C++ network library for Linux
// Copyright (c) 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Muduo team nor the names of its contributors
// may be used to endorse or promote products derived from this software
// without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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
  int timerfd = ::timerfd_create(CLOCK_MONOTONIC,
                                 TFD_NONBLOCK | TFD_CLOEXEC);
  if (timerfd < 0)
  {
    perror("Failed in timerfd_create");
    abort();
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
  timerfdChannel_.setReadCallback(
      boost::bind(&TimerQueue::timeout, this));
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
  Timestamp now(Timestamp::now());
  uint64_t howmany;
  ssize_t n = ::read(timerfd_, &howmany, sizeof howmany);
  printf("TimerQueue::timeout() timeout %" PRIu64 " at %s\n",
      howmany, now.toString().c_str());
  if (n != sizeof howmany)
  {
    fprintf(stderr, "TimerQueue::timeout() "
        "reads %zd bytes instead of 8\n", n);
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

