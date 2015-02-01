#include <muduo/base/Atomic.h>
#include <muduo/base/Condition.h>
#include <muduo/base/CurrentThread.h>
#include <muduo/base/Mutex.h>
#include <muduo/base/Thread.h>
#include <muduo/base/Timestamp.h>
#include <muduo/net/EventLoop.h>

#include <boost/ptr_container/ptr_vector.hpp>

#include <math.h>
#include <stdio.h>

using namespace muduo;
using namespace muduo::net;

int g_cycles = 0;
int g_percent = 82;
AtomicInt32 g_done;
bool g_busy = false;
MutexLock g_mutex;
Condition g_cond(g_mutex);

double busy(int cycles)
{
  double result = 0;
  for (int i = 0; i < cycles; ++i)
  {
    result += sqrt(i) * sqrt(i+1);
  }
  return result;
}

double getSeconds(int cycles)
{
  Timestamp start = Timestamp::now();
  busy(cycles);
  return timeDifference(Timestamp::now(), start);
}

void findCycles()
{
  g_cycles = 1000;
  while (getSeconds(g_cycles) < 0.001)
    g_cycles = g_cycles + g_cycles / 4;  // * 1.25
  printf("cycles %d\n", g_cycles);
}

void threadFunc()
{
  while (g_done.get() == 0)
  {
    {
    MutexLockGuard guard(g_mutex);
    while (!g_busy)
      g_cond.wait();
    }
    busy(g_cycles);
  }
  printf("thread exit\n");
}

// this is open-loop control
void load(int percent)
{
  percent = std::max(0, percent);
  percent = std::min(100, percent);

  // Bresenham's line algorithm
  int err = 2*percent - 100;
  int count = 0;

  for (int i = 0; i < 100; ++i)
  {
    bool busy = false;
    if (err > 0)
    {
      busy = true;
      err += 2*(percent - 100);
      ++count;
      // printf("%2d, ", i);
    }
    else
    {
      err += 2*percent;
    }

    {
    MutexLockGuard guard(g_mutex);
    g_busy = busy;
    g_cond.notifyAll();
    }

    CurrentThread::sleepUsec(10*1000); // 10 ms
  }
  assert(count == percent);
}

void fixed()
{
  while (true)
  {
    load(g_percent);
  }
}

void cosine()
{
  while (true)
    for (int i = 0; i < 200; ++i)
    {
      int percent = static_cast<int>((1.0 + cos(i * 3.14159 / 100)) / 2 * g_percent + 0.5);
      load(percent);
    }
}

void sawtooth()
{
  while (true)
    for (int i = 0; i <= 100; ++i)
    {
      int percent = static_cast<int>(i / 100.0 * g_percent);
      load(percent);
    }
}

int main(int argc, char* argv[])
{
  if (argc < 2)
  {
    printf("Usage: %s [fctsz] [percent] [num_threads]\n", argv[0]);
    return 0;
  }

  printf("pid %d\n", getpid());
  findCycles();

  g_percent = argc > 2 ? atoi(argv[2]) : 43;
  int numThreads = argc > 3 ? atoi(argv[3]) : 1;
  boost::ptr_vector<Thread> threads;
  for (int i = 0; i < numThreads; ++i)
  {
    threads.push_back(new Thread(threadFunc));
    threads.back().start();
  }

  switch (argv[1][0])
  {
    case 'f':
    {
      fixed();
    }
    break;

    case 'c':
    {
      cosine();
    }
    break;

    case 'z':
    {
      sawtooth();
    }
    break;

    // TODO: square and triangle waves

    default:
    break;
  }

  g_done.getAndSet(1);
  {
  MutexLockGuard guard(g_mutex);
  g_busy = true;
  g_cond.notifyAll();
  }
  for (int i = 0; i < numThreads; ++i)
  {
    threads[i].join();
  }
}
