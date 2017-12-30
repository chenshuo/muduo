// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is an internal header file, you should not include this.

#ifndef MUDUO_NET_TIMERQUEUE_H
#define MUDUO_NET_TIMERQUEUE_H

#include <set>
#include <vector>

#include <muduo/base/Mutex.h>
#include <muduo/base/Timestamp.h>
#include <muduo/net/Callbacks.h>
#include <muduo/net/Channel.h>


namespace muduo
{
namespace net
{

class EventLoop;
class Timer;
class TimerId;

///
/// A best efforts timer queue.
/// No guarantee that the callback will be on time.
///

typedef std::shared_ptr<Timer> TimerPtr;
typedef std::function<bool(TimerPtr,TimerPtr)> TimerPtrLessComp;

class TimerQueue : noncopyable
{
 public:
  explicit TimerQueue(EventLoop* loop);
  ~TimerQueue();

  ///
  /// Schedules the callback to be run at given time,
  /// repeats if @c interval > 0.0.
  ///
  /// Must be thread safe. Usually be called from other threads.
  TimerId addTimer(TimerCallback cb,
                   Timestamp when,
                   double interval);

  void cancel(TimerId timerId);

  static bool lesscomp(TimerPtr lhs, TimerPtr rhs);

 private:
  typedef std::set<TimerPtr,TimerPtrLessComp> TimerList;

  void addTimerInLoop(TimerPtr timer);
  void cancelInLoop(TimerId timerId);
  // called when timerfd alarms
  void handleRead();
  // move out all expired timers
  void getExpired(Timestamp now,std::vector<TimerPtr>&);
  void reset(std::vector<TimerPtr >& expired, Timestamp now);

  bool insert(TimerPtr &timer);

  EventLoop* loop_;
  const int timerfd_;
  Channel timerfdChannel_;

  TimerList timers_;
  TimerList timersCancle_;
};

}
}
#endif  // MUDUO_NET_TIMERQUEUE_H
