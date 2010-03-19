// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is an internal header file, you should not include this.

#ifndef MUDUO_NET_EVENTLOOPPOOL_H
#define MUDUO_NET_EVENTLOOPPOOL_H

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

class EventLoopPool : boost::noncopyable
{
 public:
  EventLoopPool(EventLoop* baseLoop);
  ~EventLoopPool();
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

#endif  // MUDUO_NET_EVENTLOOPPOOL_H
