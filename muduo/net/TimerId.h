// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is a public header file, it must only include public header files.

#ifndef MUDUO_NET_TIMERID_H
#define MUDUO_NET_TIMERID_H

#include <muduo/base/copyable.h>

namespace muduo
{
namespace net
{

class Timer;

///
/// An opaque identifier, for canceling Timer.
///
class TimerId : public muduo::copyable
{
 public:
  explicit TimerId(Timer* timer)
    : value_(timer)
  {
  }

 private:
  Timer* value_;
};

}
}

#endif  // MUDUO_NET_TIMERID_H
