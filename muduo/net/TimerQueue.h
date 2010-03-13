#ifndef MUDUO_NET_TIMERQUEUE_H
#define MUDUO_NET_TIMERQUEUE_H

#include <list>

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>

#include <muduo/base/Mutex.h>
#include <muduo/base/UtcTime.h>
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
class TimerQueue : boost::noncopyable
{
 public:
  typedef boost::function<void()> TimerCallback;

  TimerQueue(EventLoop* loop);
  ~TimerQueue();

  ///
  /// Schedules the callback to be run at given time,
  /// repeats if @c interval > 0.0.
  ///
  /// Must be thread safe.
  TimerId schedule(const TimerCallback& cb, UtcTime at, double interval);

  void cancel(TimerId timerId);

 private:
  void timeout();

  typedef std::list<Timer*> TimerList;

  EventLoop* loop_;
  const int timerfd_;
  Channel timerfdChannel_;
  MutexLock mutex_;
  TimerList timers_; // GuardedBy mutex_
};

}
}
#endif
