#include <muduo/net/EventLoop.h>

#include <muduo/base/Mutex.h>
#include <muduo/base/Thread.h>
#include <muduo/net/Channel.h>
#include <muduo/net/Poller.h>
#include <muduo/net/TimerQueue.h>

#include <boost/bind.hpp>

#include <stdio.h> // FIXME
#include <sys/eventfd.h>

using namespace muduo;
using namespace muduo::net;

namespace
{
__thread EventLoop* t_loopInThisThread = 0;

const int kPollTimeMs = 10000;

int createEventfd()
{
  int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (evtfd < 0)
  {
    perror("Failed in eventfd");
    abort();
  }
  return evtfd;
}
}

EventLoop::EventLoop()
  : looping_(false),
    quit_(false),
    threadId_(CurrentThread::tid()),
    poller_(Poller::newDefaultPoller()),
    timerQueue_(new TimerQueue(this)),
    wakeupFd_(createEventfd()),
    wakeupChannel_(new Channel(this, wakeupFd_))
{
  printf("EventLoop created in thread %d\n", threadId_);
  if (t_loopInThisThread)
  {
    fprintf(stderr, "Another EventLoop %p exists in this thread %d\n",
        t_loopInThisThread, threadId_);
    abort();
  }
  else
  {
    t_loopInThisThread = this;
  }
  wakeupChannel_->setReadCallback(
      boost::bind(&EventLoop::wakedup, this));
  // we are always reading the wakeupfd, like the old pipe(2) way.
  wakeupChannel_->set_events(Channel::kReadEvent);
  updateChannel(get_pointer(wakeupChannel_));
}

EventLoop::~EventLoop()
{
  ::close(wakeupFd_);
  t_loopInThisThread = NULL;
}

void EventLoop::loop()
{
  assert(!looping_);
  assertInLoopThread();
  looping_ = true;
  while (!quit_)
  {
    activeChannels_.clear();
    poller_->poll(kPollTimeMs, &activeChannels_);
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

void EventLoop::wakeup()
{
  uint64_t one = 1;
  ssize_t n = ::write(wakeupFd_, &one, sizeof one);
  if (n != sizeof one)
  {
    fprintf(stderr, "EventLoop::wakeup() write %zd bytes instead of 8\n", n);
  }
}

void EventLoop::runInLoop(const Functor& cb)
{
  if (threadId_ == CurrentThread::tid())
  {
    cb();
  }
  else
  {
    abort();
  }
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

void EventLoop::updateChannel(Channel* channel)
{
  assert(channel->getLoop() == this);
  assertInLoopThread();
  poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel* channel)
{
  assert(channel->getLoop() == this);
  // poller_->removeChannel(channel);
}

void EventLoop::assertInLoopThread()
{
  assert(threadId_ == CurrentThread::tid());
}

void EventLoop::wakedup()
{
  // what's up
}

