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

#ifndef MUDUO_NET_THREADMODEL_H
#define MUDUO_NET_THREADMODEL_H

#include <muduo/base/Condition.h>
#include <muduo/base/Mutex.h>

#include <vector>
#include <boost/noncopyable.hpp>

namespace muduo
{

class Thread;

namespace net
{

class EventLoop;

class ThreadModel : boost::noncopyable
{
 public:
  ThreadModel(EventLoop* baseLoop);
  ~ThreadModel();
  void setThreadNum(int numThreads) { numThreads_ = numThreads; }
  void start();
  EventLoop* getNextLoop();

 private:
  void threadFunc();

  EventLoop* baseLoop_;
  bool started_;
  bool exiting_;
  int numThreads_;
  int next_;
  std::vector<Thread*> threads_;
  MutexLock mutex_;
  Condition cond_;
  std::vector<EventLoop*> loopPool_; // @GuardedBy mutex_
};

}
}

#endif  // MUDUO_NET_THREADMODEL_H
