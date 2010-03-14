#include <muduo/net/EPollPoller.h>

#include <muduo/net/Channel.h>

#include <assert.h>
#include <poll.h>
#include <sys/epoll.h>

#include <boost/static_assert.hpp>
using namespace muduo;
using namespace muduo::net;

// On Linux, the constants of poll(2) and epoll(4)
// are expected to be the same.
BOOST_STATIC_ASSERT(EPOLLIN == POLLIN);
BOOST_STATIC_ASSERT(EPOLLPRI == POLLPRI);
BOOST_STATIC_ASSERT(EPOLLOUT == POLLOUT);
BOOST_STATIC_ASSERT(EPOLLRDHUP == POLLRDHUP);
BOOST_STATIC_ASSERT(EPOLLERR == POLLERR);
BOOST_STATIC_ASSERT(EPOLLHUP == POLLHUP);

EPollPoller::~EPollPoller()
{
}

void EPollPoller::poll(int timeoutMs, ChannelList* activeChannels)
{
}

