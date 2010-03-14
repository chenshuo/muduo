#include <muduo/base/Thread.h>

#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <linux/unistd.h>

namespace
{
__thread pid_t t_cachedTid = 0;

pid_t gettid()
{
  return static_cast<pid_t>(::syscall(SYS_gettid));
}

}

using namespace muduo;

pid_t CurrentThread::tid()
{
  if (t_cachedTid == 0)
  {
    t_cachedTid = gettid();
  }
  return t_cachedTid;
}

Thread::Thread(const ThreadFunc& func)
  : started_(false),
    pthreadId_(0),
    tid_(0),
    func_(func)
{
}

Thread::~Thread()
{
}

void Thread::start()
{
  assert(!started_);
  started_ = true;
  pthread_create(&pthreadId_, NULL, &startThread, this);
}

void Thread::join()
{
  assert(started_);
  pthread_join(pthreadId_, NULL);
}

void* Thread::startThread(void* obj)
{
  Thread* thread = static_cast<Thread*>(obj);
  thread->tid_ = CurrentThread::tid();
  thread->func_();
  return NULL;
}

