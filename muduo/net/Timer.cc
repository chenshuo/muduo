// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include <muduo/net/Timer.h>

using namespace muduo;
using namespace muduo::net;

void Timer::restart(Timestamp now)
{
  if (repeat_)
  {
    expiration_ = addTime(now, interval_);
  }
  else
  {
    expiration_ = Timestamp::invalid();
  }
}
