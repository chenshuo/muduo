// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "muduo/net/Poller.h"
#include "muduo/net/poller/PollPoller.h"

#ifdef __linux__
#include "muduo/net/poller/EPollPoller.h"
#elif defined(__APPLE__)
#include "muduo/net/poller/KQueuePoller.h"
#endif

#include <stdlib.h>

using namespace muduo::net;

Poller* Poller::newDefaultPoller(EventLoop* loop)
{
  if (::getenv("MUDUO_USE_POLL"))
  {
    return new PollPoller(loop);
  }
#ifdef __linux__
  else
  {
    return new EPollPoller(loop);
  }
#elif defined(__APPLE__)
  else
  {
    return new KQueuePoller(loop);
  }
#else
  else
  {
    return new PollPoller(loop);
  }
#endif
}