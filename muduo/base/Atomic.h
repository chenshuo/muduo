#ifndef MUDUO_BASE_ATOMIC_H
#define MUDUO_BASE_ATOMIC_H

#include <boost/noncopyable.hpp>

namespace muduo
{

class AtomicInt64 : boost::noncopyable
{
 public:
  AtomicInt64()
    : value_(0)
  {
  }

  int64_t get()
  {
    return value_;
  }

  int64_t addAndGet(int64_t x)
  {
    return __sync_add_and_fetch(&value_, x);
  }

  int64_t incrementAndGet()
  {
    return addAndGet(1);
  }

  int64_t getAndSet(int64_t newValue)
  {
    return __sync_lock_test_and_set(&value_, newValue);
  }

 private:
  volatile int64_t value_;
};

}
#endif  // MUDUO_BASE_ATOMIC_H
