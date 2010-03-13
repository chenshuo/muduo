#include <muduo/net/Timer.h>

using namespace muduo;
using namespace muduo::net;

void Timer::restart(UtcTime now)
{
  if (repeat_)
  {
    expiration_ = addTime(now, interval_);
  }
  else
  {
    expiration_ = UtcTime::invalid();
  }
}
