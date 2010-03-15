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

#include <muduo/net/ThreadModel.h>

#include <muduo/base/Thread.h>
#include <muduo/net/EventLoop.h>

#include <boost/bind.hpp>

using namespace muduo;
using namespace muduo::net;


ThreadModel::ThreadModel(EventLoop* baseLoop)
  : baseLoop_(baseLoop),
    started_(false),
    exiting_(false),
    numThreads_(0),
    next_(0),
    mutex_(),
    cond_(mutex_)
{
}

ThreadModel::~ThreadModel()
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

void ThreadModel::start()
{
  assert(!started_);
  baseLoop_->assertInLoopThread();

  started_ = true;

  Thread::ThreadFunc func = boost::bind(&ThreadModel::threadFunc, this);

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

EventLoop* ThreadModel::getNextLoop()
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

void ThreadModel::threadFunc()
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

