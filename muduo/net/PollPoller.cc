#include <muduo/net/PollPoller.h>

#include <poll.h>

using namespace muduo;
using namespace muduo::net;

PollPoller::~PollPoller()
{
}

void PollPoller::poll(int timeoutMs)
{
  // make a copy
  PollFdList pollfds(pollfds_);
  int numEvents = ::poll(&*pollfds.begin(), pollfds.size(), timeoutMs);

  for (PollFdList::iterator pfd = pollfds.begin();
      pfd != pollfds.end() && numEvents > 0; ++pfd)
  {
    if (pfd->revents > 0)
    {
      --numEvents;
      /*
      NetLogDebug << "fd:revents " << pfd->fd << " : " << pfd->revents << NetSend;
      // assert(0 <= pfd->fd && pfd->fd < static_cast<int>(channels_.size()));
      Channel* channel = channels_[pfd->fd];
      assert(channel->fd() == pfd->fd);
      channel->handle(pfd->revents);
      */
    }
  }
}

