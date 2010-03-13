#include <muduo/base/Thread.h>

#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <linux/unistd.h>

namespace
{
  __thread pid_t t_tid = 0;

  pid_t gettid()
  {
    return static_cast<pid_t>(::syscall(SYS_gettid));
  }

  void* startThread(void* cb)
  {
    muduo::Thread::ThreadFunc* func = static_cast<muduo::Thread::ThreadFunc*>(cb);
    t_tid = gettid();
    (*func)();
    return NULL;
  }
}

using namespace muduo;

Thread::Thread(const ThreadFunc& func)
  : ptid_(0),
    func_(func)
{
}

Thread::~Thread()
{
}

void Thread::start()
{
  pthread_create(&ptid_, NULL, &startThread, &func_);
}

void Thread::join()
{
  pthread_join(ptid_, NULL);
}

pid_t CurrentThread::tid()
{
  if (t_tid == 0) {
    t_tid = gettid();
  }
  return t_tid;
}

