#include <muduo/base/Mutex.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/EventLoopThread.h>

#include <iostream>
#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>

class Printer : boost::noncopyable
{
 public:
  Printer(muduo::net::EventLoop* loop1, muduo::net::EventLoop* loop2)
    : loop1_(loop1),
      loop2_(loop2),
      count_(0)
  {
    loop1_->runAfter(1, boost::bind(&Printer::print1, this));
    loop2_->runAfter(1, boost::bind(&Printer::print2, this));
  }

  ~Printer()
  {
    std::cout << "Final count is " << count_ << "\n";
  }

  void print1()
  {
    muduo::MutexLockGuard lock(mutex_);
    if (count_ < 10)
    {
      std::cout << "Timer 1: " << count_ << "\n";
      ++count_;

      loop1_->runAfter(1, boost::bind(&Printer::print1, this));
    }
    else
    {
      loop1_->quit();
    }
  }

  void print2()
  {
    muduo::MutexLockGuard lock(mutex_);
    if (count_ < 10)
    {
      std::cout << "Timer 2: " << count_ << "\n";
      ++count_;

      loop2_->runAfter(1, boost::bind(&Printer::print2, this));
    }
    else
    {
      loop2_->quit();
    }
  }

private:

  muduo::MutexLock mutex_;
  muduo::net::EventLoop* loop1_;
  muduo::net::EventLoop* loop2_;
  int count_;
};

int main()
{
  boost::scoped_ptr<Printer> printer;  // make sure printer lives longer than loops, to avoid
                                       // race condition of calling print2() on destructed object.
  muduo::net::EventLoop loop;
  muduo::net::EventLoopThread loopThread;
  muduo::net::EventLoop* loopInAnotherThread = loopThread.startLoop();
  printer.reset(new Printer(&loop, loopInAnotherThread));
  loop.loop();
}

