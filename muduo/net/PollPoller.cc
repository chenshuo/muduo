#include <muduo/net/PollPoller.h>

#include <muduo/net/Channel.h>

#include <assert.h>
#include <stdio.h>

using namespace muduo;
using namespace muduo::net;

PollPoller::~PollPoller()
{
}

void PollPoller::poll(int timeoutMs, ChannelList* activeChannels)
{
  // XXX pollfds_ shouldn't change
  int numEvents = ::poll(&*pollfds_.begin(), pollfds_.size(), timeoutMs);
  if (numEvents > 0)
  {
    fillActiveChannels(numEvents, activeChannels);
  }
  else if (numEvents == 0)
  {
    printf("nothing\n");
  }
  else
  {
    perror("PollPoller::poll");
  }

}

void PollPoller::fillActiveChannels(int numEvents, ChannelList* activeChannels) const
{
  for (PollFdList::const_iterator pfd = pollfds_.begin();
      pfd != pollfds_.end() && numEvents > 0; ++pfd)
  {
    if (pfd->revents > 0)
    {
      --numEvents;
      ChannelMap::const_iterator ch = channels_.find(pfd->fd);
      assert(ch != channels_.end());
      Channel* channel = ch->second;
      assert(channel->fd() == pfd->fd);
      channel->set_revents(pfd->revents);
      // pfd->revents = 0;
      activeChannels->push_back(channel);
    }
  }
}

void PollPoller::updateChannel(Channel* channel)
{
  // assert(channel->getLoop()

  if (channel->index() < 0)
  {
    // a new one, add to pollfds_
    assert(channels_.find(channel->fd()) == channels_.end());
    struct pollfd pfd;
    pfd.fd = channel->fd();
    pfd.events = static_cast<short>(channel->events());
    pfd.revents = 0;
    pollfds_.push_back(pfd);
    channel->set_index(static_cast<int>(pollfds_.size())-1);
    channels_[pfd.fd] = channel;
  }
  else
  {
    // update existing one
    assert(channels_.find(channel->fd()) != channels_.end());
    assert(channels_[channel->fd()] == channel);
    int idx = channel->index();
    assert(0 <= idx && idx < static_cast<int>(pollfds_.size()));
    struct pollfd& pfd = pollfds_[idx];
    assert(pfd.fd == channel->fd() || pfd.fd == -1);
    pfd.events = static_cast<short>(channel->events());
    if (pfd.events == 0)
    {
      // ignore this pollfd
      pfd.fd = -1;
      printf("set pfd.fd=-1 for fd=%d\n", channel->fd());
    }
  }

}

