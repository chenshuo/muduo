#ifndef MUDUO_NET_TIMER_H
#define MUDUO_NET_TIMER_H

#include <map>

#include <boost/noncopyable.hpp>

#include <muduo/base/UtcTime.h>
#include <muduo/net/Callbacks.h>

namespace muduo
{
namespace net
{
///
/// Internal class for timer event.
///
class Timer : boost::noncopyable
{
 public:
  Timer(const TimerCallback& cb, UtcTime when, double interval)
    : cb_(cb),
      expiration_(when),
      interval_(interval),
      repeat_(interval > 0.0)
  { }

  void run() const
  {
    cb_();
  }

  UtcTime expiration() const  { return expiration_; }
  bool repeat() const { return repeat_; }

  void restart(UtcTime now);

 private:
  const TimerCallback cb_;
  UtcTime expiration_;
  const double interval_;
  const bool repeat_;
};
}
}
#endif  // MUDUO_NET_TIMER_H
