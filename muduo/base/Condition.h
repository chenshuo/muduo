// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef MUDUO_BASE_CONDITION_H
#define MUDUO_BASE_CONDITION_H

#include <muduo/base/Mutex.h>

#include <boost/noncopyable.hpp>
#include <pthread.h>

namespace muduo
{

class Condition : boost::noncopyable
{
 public:
  explicit Condition(MutexLock& mutex)
    : mutex_(mutex)
  {
    int ret = pthread_cond_init(&pcond_, NULL);
    assert(ret == 0); (void) ret;
  }

  ~Condition()
  {
    int ret = pthread_cond_destroy(&pcond_);
    assert(ret == 0); (void) ret;
  }

  void wait()
  {
    MutexLock::UnassignGuard ug(mutex_);
    int ret = pthread_cond_wait(&pcond_, mutex_.getPthreadMutex());
    assert(ret == 0); (void) ret;
  }

  // returns true if time out, false otherwise.
  bool waitForSeconds(int seconds);

  void notify()
  {
    int ret = pthread_cond_signal(&pcond_);
    assert(ret == 0); (void) ret;
  }

  void notifyAll()
  {
    int ret = pthread_cond_broadcast(&pcond_);
    assert(ret == 0); (void) ret;
  }

 private:
  MutexLock& mutex_;
  pthread_cond_t pcond_;
};

}
#endif  // MUDUO_BASE_CONDITION_H
