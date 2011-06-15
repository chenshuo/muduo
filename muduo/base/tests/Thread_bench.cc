#include <muduo/base/Thread.h>
#include <muduo/base/Timestamp.h>

#include <string>
#include <boost/bind.hpp>
#include <stdio.h>
#include <sys/wait.h>

void threadFunc()
{
  //printf("tid=%d\n", muduo::CurrentThread::tid());
}

void forkBench()
{
  sleep(10);
  muduo::Timestamp start(muduo::Timestamp::now());
  int kProcesses = 10*1000;

  for (int i = 0; i < kProcesses; ++i)
  {
    pid_t child = fork();
    if (child == 0) {
      exit(0);
    } else {
      waitpid(child, NULL, 0);
    }
  }

  double timeUsed = timeDifference(muduo::Timestamp::now(), start);
  printf("process creation time used %f us\n", timeUsed*1000000/kProcesses);
  printf("number of created processes %d\n", kProcesses);
}

int main(int argc, char* argv[])
{
  printf("pid=%d, tid=%d\n", ::getpid(), muduo::CurrentThread::tid());
  muduo::Timestamp start(muduo::Timestamp::now());

  int kThreads = 100*1000;
  for (int i = 0; i < kThreads; ++i)
  {
    muduo::Thread t1(threadFunc);
    t1.start();
    t1.join();
  }

  double timeUsed = timeDifference(muduo::Timestamp::now(), start);
  printf("thread creation time %f us\n", timeUsed*1000000/kThreads);
  printf("number of created threads %d\n", muduo::Thread::numCreated());

  forkBench();
}
