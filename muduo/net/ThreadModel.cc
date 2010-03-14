// Copyright 2010 Shuo Chen (chenshuo at chenshuo dot com)
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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

