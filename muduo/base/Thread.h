#ifndef MUDUO_BASE_THREAD_H
#define MUDUO_BASE_THREAD_H

#include <pthread.h>
#include <boost/function.hpp>

namespace muduo
{

class Thread
{
 public:
  typedef boost::function<void ()> ThreadFunc;

  explicit Thread(const ThreadFunc&);
  ~Thread();

  void start();
  void join();

  bool started() const { return started_; }
  pthread_t pthreadId() const { return pthreadId_; }
  pid_t tid() const { return tid_; }

 private:
  static void* startThread(void* thread);

  bool       started_;
  pthread_t  pthreadId_;
  pid_t      tid_;
  ThreadFunc func_;
};

namespace CurrentThread
{
  pid_t tid();
}

}

#endif
