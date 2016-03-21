// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include <muduo/base/Condition.h>

#include <errno.h>

// returns true if time out, false otherwise.
bool muduo::Condition::waitForSeconds(double seconds) {
  struct timespec abstime;
  // FIXME: use CLOCK_MONOTONIC or CLOCK_MONOTONIC_RAW to prevent time rewind.
  clock_gettime(CLOCK_REALTIME, &abstime);

  const int64_t kNanoSecondsPerSecond = 1e9;
  int64_t nanoseconds = static_cast<int64_t>(seconds * kNanoSecondsPerSecond);
  if (1e5 > nanoseconds){
    nanoseconds = 1e5;
  }

  abstime.tv_sec += static_cast<time_t>(nanoseconds / kNanoSecondsPerSecond);
  abstime.tv_nsec += static_cast<long>(nanoseconds % kNanoSecondsPerSecond);

  MutexLock::UnassignGuard ug(mutex_);
  return ETIMEDOUT
      == pthread_cond_timedwait(&pcond_, mutex_.getPthreadMutex(), &abstime);
}