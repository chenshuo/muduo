#include <muduo/net/EPollPoller.h>

#include <muduo/net/Channel.h>

#include <assert.h>
#include <poll.h>
#include <sys/epoll.h>

using namespace muduo;
using namespace muduo::net;

namespace
{
  // On Linux, the constants of poll(2) and epoll(4)
  // are expected to be the same.
  char poll_epoll_event_diff_in[EPOLLIN == POLLIN ? 1 : -1];
  char poll_epoll_event_diff_pri[EPOLLPRI == POLLPRI ? 1 : -1];
  char poll_epoll_event_diff_out[EPOLLOUT == POLLOUT ? 1 : -1];
  char poll_epoll_event_diff_rdhup[EPOLLRDHUP == POLLRDHUP ? 1 : -1];
  char poll_epoll_event_diff_err[EPOLLERR == POLLERR ? 1 : -1];
  char poll_epoll_event_diff_hup[EPOLLHUP == POLLHUP ? 1 : -1];

  void nowarning()
  {
    (void)poll_epoll_event_diff_in;
    (void)poll_epoll_event_diff_out;
    (void)poll_epoll_event_diff_pri;
    (void)poll_epoll_event_diff_rdhup;
    (void)poll_epoll_event_diff_err;
    (void)poll_epoll_event_diff_hup;
  }
}

EPollPoller::~EPollPoller()
{
}

void EPollPoller::poll(int timeoutMs, ChannelList* activeChannels)
{
}

