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
    value_ += x;
    return value_;
  }

 private:
  int64_t value_;
};

}
#endif  // MUDUO_BASE_ATOMIC_H
