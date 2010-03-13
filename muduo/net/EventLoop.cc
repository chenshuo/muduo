#include <muduo/net/EventLoop.h>

#include <muduo/base/Mutex.h>
#include <muduo/base/Thread.h>
#include <muduo/net/Channel.h>
#include <muduo/net/Poller.h>
#include <muduo/net/TimerQueue.h>

using namespace muduo;
using namespace muduo::net;

EventLoop::EventLoop()
  : poller_(Poller::newDefaultPoller()),
    timerQueue_(new TimerQueue(this)),
    looping_(false),
    quit_(false),
    thread_(CurrentThread::tid())
{
  init();
}

EventLoop::~EventLoop()
{
}

void EventLoop::loop()
{
  assert(!looping_);
  looping_ = true;
  while (!quit_)
  {
    activeChannels_.clear();
    poller_->poll(1000, &activeChannels_);
    for (ChannelList::iterator it = activeChannels_.begin();
        it != activeChannels_.end(); ++it)
    {
      (*it)->handle_event();
    }
  }
  looping_ = false;
}

void EventLoop::quit()
{
  quit_ = true;
}

void EventLoop::updateChannel(Channel* channel)
{
  assert(channel->getLoop() == this);
  // channel->set_loop(this);
  // poller_->addChannel(channel);
}

void EventLoop::removeChannel(Channel* channel)
{
  assert(channel->getLoop() == this);
  // poller_->removeChannel(channel);
}

void EventLoop::init()
{
}

TimerId EventLoop::runAt(const UtcTime& time, const TimerCallback& cb)
{
  return timerQueue_->schedule(cb, time, 0.0);
}

TimerId EventLoop::runAfter(double delay, const TimerCallback& cb)
{
  UtcTime time(addTime(UtcTime::now(), delay));
  return runAt(time, cb);
}

TimerId EventLoop::runEvery(double interval, const TimerCallback& cb)
{
  UtcTime time(addTime(UtcTime::now(), interval));
  return timerQueue_->schedule(cb, time, interval);
}

