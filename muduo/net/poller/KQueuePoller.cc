// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "muduo/net/poller/KQueuePoller.h"

#include "muduo/base/Logging.h"
#include "muduo/net/Channel.h"

#include <assert.h>
#include <errno.h>
#include <poll.h>
#include <sys/event.h>
#include <unistd.h>

using namespace muduo;
using namespace muduo::net;

// On macOS, the constants of poll(2) are used directly.
// kqueue uses EVFILT_READ/EVFILT_WRITE instead of events flags,
// but the Channel class uses POLLIN/POLLOUT etc.
// We map them in update() function.

namespace
{
const int kNew = -1;
const int kAdded = 1;
const int kDeleted = 2;
}  // namespace

KQueuePoller::KQueuePoller(EventLoop* loop)
  : Poller(loop),
    kqfd_(::kqueue()),
    events_(kInitEventListSize)
{
  if (kqfd_ < 0)
  {
    LOG_SYSFATAL << "KQueuePoller::KQueuePoller";
  }
}

KQueuePoller::~KQueuePoller()
{
  ::close(kqfd_);
}

Timestamp KQueuePoller::poll(int timeoutMs, ChannelList* activeChannels)
{
  LOG_TRACE << "fd total count " << channels_.size();

  struct timespec ts;
  ts.tv_sec = timeoutMs / 1000;
  ts.tv_nsec = (timeoutMs % 1000) * 1000000;

  int numEvents = ::kevent(kqfd_,
                           NULL, 0,
                           &*events_.begin(),
                           static_cast<int>(events_.size()),
                           &ts);
  int savedErrno = errno;
  Timestamp now(Timestamp::now());

  if (numEvents > 0)
  {
    LOG_TRACE << numEvents << " events happened";
    fillActiveChannels(numEvents, activeChannels);
    if (implicit_cast<size_t>(numEvents) == events_.size())
    {
      events_.resize(events_.size() * 2);
    }
  }
  else if (numEvents == 0)
  {
    LOG_TRACE << "nothing happened";
  }
  else
  {
    if (savedErrno != EINTR)
    {
      errno = savedErrno;
      LOG_SYSERR << "KQueuePoller::poll()";
    }
  }

  return now;
}

void KQueuePoller::fillActiveChannels(int numEvents,
                                       ChannelList* activeChannels) const
{
  assert(implicit_cast<size_t>(numEvents) <= events_.size());

  for (int i = 0; i < numEvents; ++i)
  {
    Channel* channel = static_cast<Channel*>(events_[i].udata);
#ifndef NDEBUG
    int fd = channel->fd();
    ChannelMap::const_iterator it = channels_.find(fd);
    assert(it != channels_.end());
    assert(it->second == channel);
#endif

    int revents = 0;
    if (events_[i].flags & EV_ERROR)
    {
      revents |= POLLERR;
    }
    if (events_[i].flags & EV_EOF)
    {
      revents |= POLLHUP;
    }
    if (events_[i].filter == EVFILT_READ)
    {
      revents |= POLLIN;
    }
    if (events_[i].filter == EVFILT_WRITE)
    {
      revents |= POLLOUT;
    }

    channel->set_revents(revents);
    activeChannels->push_back(channel);
  }
}

void KQueuePoller::updateChannel(Channel* channel)
{
  Poller::assertInLoopThread();
  const int index = channel->index();
  LOG_TRACE << "fd = " << channel->fd()
    << " events = " << channel->events() << " index = " << index;

  if (index == kNew || index == kDeleted)
  {
    int fd = channel->fd();
    if (index == kNew)
    {
      assert(channels_.find(fd) == channels_.end());
      channels_[fd] = channel;
    }
    else  // index == kDeleted
    {
      assert(channels_.find(fd) != channels_.end());
      assert(channels_[fd] == channel);
    }

    channel->set_index(kAdded);
    update(EV_ADD, channel);
  }
  else
  {
    int fd = channel->fd();
    (void)fd;
    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] == channel);
    assert(index == kAdded);

    if (channel->isNoneEvent())
    {
      update(EV_DELETE, channel);
      channel->set_index(kDeleted);
    }
    else
    {
      // kqueue doesn't have EV_MOD, use DELETE + ADD
      update(EV_DELETE, channel);
      update(EV_ADD, channel);
    }
  }
}

void KQueuePoller::removeChannel(Channel* channel)
{
  Poller::assertInLoopThread();
  int fd = channel->fd();
  LOG_TRACE << "fd = " << fd;

  assert(channels_.find(fd) != channels_.end());
  assert(channels_[fd] == channel);
  assert(channel->isNoneEvent());

  int index = channel->index();
  assert(index == kAdded || index == kDeleted);

  size_t n = channels_.erase(fd);
  (void)n;
  assert(n == 1);

  if (index == kAdded)
  {
    update(EV_DELETE, channel);
  }

  channel->set_index(kNew);
}

void KQueuePoller::update(int operation, Channel* channel)
{
  struct kevent ev[2];
  int n = 0;

  if (channel->events() & POLLIN)
  {
    EV_SET(&ev[n++], channel->fd(), EVFILT_READ, operation, 0, 0, channel);
  }
  if (channel->events() & POLLOUT)
  {
    EV_SET(&ev[n++], channel->fd(), EVFILT_WRITE, operation, 0, 0, channel);
  }

  int fd = channel->fd();
  LOG_TRACE << "kqueue op = " << operationToString(operation)
    << " fd = " << fd << " event = { " << channel->eventsToString() << " }";

  if (n > 0)
  {
    if (::kevent(kqfd_, ev, n, NULL, 0, NULL) < 0)
    {
      if (operation == EV_DELETE)
      {
        LOG_SYSERR << "kevent op =" << operationToString(operation) << " fd =" << fd;
      }
      else
      {
        LOG_SYSFATAL << "kevent op =" << operationToString(operation) << " fd =" << fd;
      }
    }
  }
}

const char* KQueuePoller::operationToString(int op)
{
  switch (op)
  {
    case EV_ADD:
      return "ADD";
    case EV_DELETE:
      return "DEL";
    default:
      assert(false && "ERROR op");
      return "Unknown Operation";
  }
}