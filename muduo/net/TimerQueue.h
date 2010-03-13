#ifndef MUDUO_NET_TIMERQUEUE_H
#define MUDUO_NET_TIMERQUEUE_H

#include <list>

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>

#include <muduo/base/UtcTime.h>

namespace muduo
{
namespace net
{

class Timer;
class TimerId;

class TimerQueue : boost::noncopyable
{
 public:
  typedef boost::function<void()> TimerCallback;

  TimerQueue();
  ~TimerQueue();

  void tick(UtcTime now);

  ///
  /// Schedules the callback to be run at given time,
  /// repeats if @c interval > 0.0.
  ///
  TimerId schedule(const TimerCallback& cb, UtcTime at, double interval);

  void cancel(TimerId timerId);

 private:

  // typedef std::multimap<UtcTime, Timer*> Timers;
  typedef std::list<Timer*> Timers;
  Timers timers_;
};

}
}
#endif
