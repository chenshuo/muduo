// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include <limits>
#include <muduo/base/Condition.h>

#include <errno.h>

// returns true if time out, false otherwise.
bool muduo::Condition::waitForSeconds(double seconds) {
  struct timespec abstime;
  // FIXME: use CLOCK_MONOTONIC or CLOCK_MONOTONIC_RAW to prevent time rewind.
  clock_gettime(CLOCK_REALTIME, &abstime);

  if(abstime.tv_sec + static_cast<long>(seconds) >
      std::numeric_limits<long>::max()){
    abstime.tv_sec = std::numeric_limits<long>::max();
  }else{
    abstime.tv_sec += static_cast<long>(seconds);
  }

  double nsec = (seconds - static_cast<double>(abstime.tv_sec)) *
      1000 * 1000 * 1000;
  if(abstime.tv_nsec + nsec > std::numeric_limits<long>::max()){
    abstime.tv_nsec = std::numeric_limits<long>::max();
  }else{
    abstime.tv_nsec += static_cast<long>(nsec);
  }

  MutexLock::UnassignGuard ug(mutex_);
  return ETIMEDOUT
      == pthread_cond_timedwait(&pcond_, mutex_.getPthreadMutex(), &abstime);
}