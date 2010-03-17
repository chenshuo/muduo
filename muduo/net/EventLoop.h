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
// This is a public header file, it must only include public header files.

#ifndef MUDUO_NET_EVENTLOOP_H
#define MUDUO_NET_EVENTLOOP_H

#include <vector>

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

#include <muduo/base/Mutex.h>
#include <muduo/base/Timestamp.h>
#include <muduo/net/Callbacks.h>
#include <muduo/net/TimerId.h>

namespace muduo
{
namespace net
{

class Channel;
class Poller;
class TimerQueue;

///
/// Reactor, at most one per thread.
///
/// This is an interface class, so don't expose too much details.
class EventLoop : boost::noncopyable
{
 public:
  typedef boost::function<void()> Functor;

  EventLoop();
  ~EventLoop();  // force out-line dtor, for scoped_ptr members.

  ///
  /// Loops forever.
  ///
  /// Must be called in the same thread as creation of the object.
  ///
  void loop();

  void quit();
  void wakeup();

  // timers

  /// Runs callback immediately in the loop thread.
  /// It wakes up the loop, and run the cb.
  /// If in the same loop thread, cb is run within the function.
  /// Safe to call from other threads.
  void runInLoop(const Functor& cb);
  /// Queues callback in the loop thread.
  /// Runs after finish pooling.
  /// Safe to call from other threads.
  void queueInLoop(const Functor& cb);
  ///
  TimerId runAt(const Timestamp& time, const TimerCallback& cb);
  ///
  /// Runs callback after @c delay seconds.
  /// Safe to call from other threads.
  TimerId runAfter(double delay, const TimerCallback& cb);
  ///
  /// Runs callback every @c interval seconds.
  /// Safe to call from other threads.
  TimerId runEvery(double interval, const TimerCallback& cb);
  /// Cancels the timer.
  /// Safe to call from other threads.
  void cancel(TimerId timerId);

  void updateChannel(Channel* channel);
  void removeChannel(Channel* channel);

  pid_t threadId() { return threadId_; }
  void assertInLoopThread();

 private:
  void wakedup();
  void doPendingFunctors();

  typedef std::vector<Channel*> ChannelList;

  bool looping_; /* atomic */
  bool quit_; /* atomic */
  bool eventHandling_;
  const pid_t threadId_;
  boost::scoped_ptr<Poller> poller_;
  boost::scoped_ptr<TimerQueue> timerQueue_;
  int wakeupFd_;
  // unlink in TimerQueue, which is an internal class,
  // we don't expose Channel to client.
  boost::scoped_ptr<Channel> wakeupChannel_;
  ChannelList activeChannels_;
  MutexLock mutex_;
  std::vector<Functor> pendingFunctors_; // @BuardedBy mutex_
};

}
}
#endif  // MUDUO_NET_EVENTLOOP_H
