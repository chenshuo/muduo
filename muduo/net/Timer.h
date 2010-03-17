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

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is an internal header file, you should not include this.

#ifndef MUDUO_NET_TIMER_H
#define MUDUO_NET_TIMER_H

#include <map>

#include <boost/noncopyable.hpp>

#include <muduo/base/Timestamp.h>
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
  Timer(const TimerCallback& cb, Timestamp when, double interval)
    : cb_(cb),
      expiration_(when),
      interval_(interval),
      repeat_(interval > 0.0)
  { }

  void run() const
  {
    cb_();
  }

  Timestamp expiration() const  { return expiration_; }
  bool repeat() const { return repeat_; }

  void restart(Timestamp now);

 private:
  const TimerCallback cb_;
  Timestamp expiration_;
  const double interval_;
  const bool repeat_;
};
}
}
#endif  // MUDUO_NET_TIMER_H
