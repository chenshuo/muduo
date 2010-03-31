// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include <muduo/net/EventLoopPool.h>

#include <muduo/base/Thread.h>
#include <muduo/net/EventLoop.h>

#include <boost/bind.hpp>

using namespace muduo;
using namespace muduo::net;


EventLoopPool::EventLoopPool(EventLoop* baseLoop)
  : baseLoop_(baseLoop),
    started_(false),
    exiting_(false),
    numThreads_(0),
    next_(0),
    mutex_(),
    cond_(mutex_)
{
}

EventLoopPool::~EventLoopPool()
{
  exiting_ = true;
  for (size_t i = 0; i < loopPool_.size(); ++i)
  {
    loopPool_[i]->quit();
    // Don't delete loop, it's stack variable
  }
  for (size_t i = 0; i < threads_.size(); ++i)
  {
    Thread* t = threads_[i];
    t->join();
    delete t;
  }
}

void EventLoopPool::start()
{
  assert(!started_);
  baseLoop_->assertInLoopThread();

  started_ = true;

  Thread::ThreadFunc func = boost::bind(&EventLoopPool::threadFunc, this);

  for (int i = 0; i < numThreads_; ++i)
  {
    Thread* t = new Thread(func);
    t->start();
    threads_.push_back(t);
  }

  {
    MutexLockGuard lock(mutex_);
    while (static_cast<int>(loopPool_.size()) != numThreads_)
    {
      cond_.wait();
    }
  }
}

EventLoop* EventLoopPool::getNextLoop()
{
  baseLoop_->assertInLoopThread();
  EventLoop* loop = baseLoop_;

  if (!loopPool_.empty())
  {
    // round-robin
    loop = loopPool_[next_];
    ++next_;
    if (next_ >= static_cast<int>(loopPool_.size()))
    {
      next_ = 0;
    }
  }
  return loop;
}

void EventLoopPool::threadFunc()
{
  EventLoop loop;

  {
    MutexLockGuard lock(mutex_);
    loopPool_.push_back(&loop);
    cond_.notify();
  }

  loop.loop();
  assert(exiting_);
}

