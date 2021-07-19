#include "muduo/base/BlockingQueue.h"
#include "muduo/base/CountDownLatch.h"
#include "muduo/base/Thread.h"
#include "muduo/base/Timestamp.h"

#include <map>
#include <string>
#include <vector>
#include <stdio.h>
#include <unistd.h>

// hot potato benchmarking https://en.wikipedia.org/wiki/Hot_potato
// N threads, one hot potato.
class Bench
{
 public:
  Bench(int numThreads)
    : startLatch_(numThreads),
      stopLatch_(1)
  {
    queues_.reserve(numThreads);
    threads_.reserve(numThreads);
    for (int i = 0; i < numThreads; ++i)
    {
      queues_.emplace_back(new muduo::BlockingQueue<int>());
      char name[32];
      snprintf(name, sizeof name, "work thread %d", i);
      threads_.emplace_back(new muduo::Thread(
            [this, i] { threadFunc(i); },
            muduo::string(name)));
    }
  }

  void Start()
  {
    muduo::Timestamp start = muduo::Timestamp::now();
    for (auto& thr : threads_)
    {
      thr->start();
    }
    startLatch_.wait();
    muduo::Timestamp started = muduo::Timestamp::now();
    printf("all %zd threads started, %.3fms\n",
           threads_.size(), 1e3 * timeDifference(started, start));
  }

  void Run()
  {
    muduo::Timestamp start = muduo::Timestamp::now();
    const int rounds = 100003;
    queues_[0]->put(rounds);

    auto done = done_.take();
    double elapsed = timeDifference(done.second, start);
    printf("thread id=%d done, total %.3fms, %.3fus / round\n",
           done.first, 1e3 * elapsed, 1e6 * elapsed / rounds);
  }

  void Stop()
  {
    muduo::Timestamp stop = muduo::Timestamp::now();
    for (const auto& queue : queues_)
    {
      queue->put(-1);
    }
    for (auto& thr : threads_)
    {
      thr->join();
    }

    muduo::Timestamp t2 = muduo::Timestamp::now();
    printf("all %zd threads joined, %.3fms\n",
           threads_.size(), 1e3 * timeDifference(t2, stop));
  }

 private:
  void threadFunc(int id)
  {
    startLatch_.countDown();

    muduo::BlockingQueue<int>* input = queues_[id].get();
    muduo::BlockingQueue<int>* output = queues_[(id+1) % queues_.size()].get();
    while (true)
    {
      int value = input->take();
      if (value > 0)
      {
        output->put(value - 1);
        if (verbose_)
        {
          // printf("thread %d, got %d\n", id, value);
        }
        continue;
      }

      if (value == 0)
      {
        done_.put(std::make_pair(id, muduo::Timestamp::now()));
      }
      break;
    }
  }

  using TimestampQueue = muduo::BlockingQueue<std::pair<int, muduo::Timestamp>>;
  TimestampQueue done_;
  muduo::CountDownLatch startLatch_, stopLatch_;
  std::vector<std::unique_ptr<muduo::BlockingQueue<int>>> queues_;
  std::vector<std::unique_ptr<muduo::Thread>> threads_;
  const bool verbose_ = true;
};

int main(int argc, char* argv[])
{
  int threads = argc > 1 ? atoi(argv[1]) : 1;

  printf("sizeof BlockingQueue = %zd\n", sizeof(muduo::BlockingQueue<int>));
  printf("sizeof deque<int> = %zd\n", sizeof(std::deque<int>));
  Bench t(threads);
  t.Start();
  t.Run();
  t.Stop();
  // exit(0);
}
