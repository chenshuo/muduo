// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "muduo/net/EventLoopThread.h"
#include "muduo/net/Channel.h"

#include "muduo/net/EventLoop.h"


#include "muduo/net/SocketsOps.h"
#include "muduo/base/Logging.h"
#include <sys/eventfd.h>
#include <unistd.h>

using namespace muduo;
using namespace muduo::net;
namespace 
{
int createEventfd()
{
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0)
    {
        LOG_SYSERR << "Failed in eventfd";
        abort();
    }
    return evtfd;
}
};

EventLoopThread::EventLoopThread(const ThreadInitCallback& cb,
                                 const string& name)
  : loop_(NULL),
    exiting_(false),
    thread_(std::bind(&EventLoopThread::threadFunc, this), name),
    mutex_(),
    cond_(mutex_),
    callback_(cb)
{
}

EventLoopThread::~EventLoopThread()
{
  exiting_ = true;
  if (loop_ != NULL) // not 100% race-free, eg. threadFunc could be running callback_.
  {
    // still a tiny chance to call destructed object, if threadFunc exits just now.
    // but when EventLoopThread destructs, usually programming is exiting anyway.
    loop_->quit();
    thread_.join();
  }
}

EventLoop* EventLoopThread::startLoop()
{
  assert(!thread_.started());
  thread_.start();

  EventLoop* loop = NULL;
  {
    MutexLockGuard lock(mutex_);
    while (loop_ == NULL)
    {
      cond_.wait();
    }
    loop = loop_;
  }

  return loop;
}

void EventLoopThread::handleRead()
{

    MutexLockGuard lock(mutex_);
    cond_.notify();
}

void EventLoopThread::threadFunc()
{
  EventLoop loop;

  if (callback_)
  {
    callback_(&loop);
  }

  {
    MutexLockGuard lock(mutex_);
    loop_ = &loop;
  }
  int  loopFd_= createEventfd();
  Channel loopReady(loop_,loopFd_);
  loopReady.setReadCallback(std::bind(&EventLoopThread::handleRead, this));
  loopReady.enableReading();
  uint64_t one = 1;
  ssize_t n = sockets::write(loopFd_, &one, sizeof one);
  if (n != sizeof one)
  {
    LOG_ERROR << "EventLoopThread::wakeup() writes " << n << " bytes instead of 8";
  }

  loop.loop();
  //assert(exiting_);
  MutexLockGuard lock(mutex_);
  loop_ = NULL;

  loopReady.disableAll();
  loopReady.remove();
  ::close(loopFd_);

}

