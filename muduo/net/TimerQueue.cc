#include <muduo/net/TimerQueue.h>

#include <muduo/net/Timer.h>
#include <muduo/net/TimerId.h>

using namespace muduo;
using namespace muduo::net;

TimerQueue::TimerQueue()
{
}

TimerQueue::~TimerQueue()
{
  /*
  for (Alarms::iterator it(alarms_.begin());
      it != alarms_.end();
      ++it)
  {
    delete it->second;
  }
  */
}

void TimerQueue::tick(UtcTime now)
{
  /*
  while (!alarms_.empty())
  {
    Alarms::iterator head(alarms_.begin());
    if (head->first.after(now))
    {
      break;
    }
    else
    {
      Alarm* alarm = head->second;
      alarm->run();
      UtcTime next = alarm->getNextTime(now);
      if (next.valid())
      {
        alarms_.insert(std::make_pair(next, alarm));
      }
      else
      {
        delete alarm;
      }
      alarms_.erase(head);
    }
  }
  return alarms_.empty() ? UtcTime() : alarms_.begin()->first;
  */
}

TimerId TimerQueue::schedule(const TimerCallback& cb, UtcTime at, double interval)
{
  Timer* timer = new Timer(cb, at, interval);
  timers_.push_front(timer);
  return TimerId(timer);
}

