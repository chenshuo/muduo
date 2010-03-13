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

 private:

  pthread_t  ptid_;
  ThreadFunc func_;
};

namespace CurrentThread
{
  pid_t tid();
}

}

#endif
