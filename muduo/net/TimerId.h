#ifndef MUDUO_NET_TIMERID_H
#define MUDUO_NET_TIMERID_H

namespace muduo
{
namespace net
{

class Timer;

class TimerId
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

#endif
