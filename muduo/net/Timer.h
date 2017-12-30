// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is an internal header file, you should not include this.

#ifndef MUDUO_NET_TIMER_H
#define MUDUO_NET_TIMER_H

#include <muduo/base/Atomic.h>
#include <muduo/base/Timestamp.h>
#include <muduo/net/Callbacks.h>

namespace muduo
{
namespace net
{
///
/// Internal class for timer event.
///
class Timer : noncopyable
{
public:
  Timer(TimerCallback cb, Timestamp when, double interval)
      : callback_(std::move(cb)),
        expiration_(when),
        interval_(interval),
        repeat_(interval > 0.0)
  {
  }

  void run() const
  {
    if (callback_)
    {
      callback_();
    }
  }

  Timestamp expiration() const { return expiration_; }
  bool repeat() const { return repeat_; }
  void restart(Timestamp now);
private:
  const TimerCallback callback_;
  Timestamp expiration_;
  const double interval_;
  const bool repeat_;
};

}
}
#endif // MUDUO_NET_TIMER_H
