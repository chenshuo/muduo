#include <muduo/net/EPollPoller.h>

#include <muduo/net/Channel.h>

#include <assert.h>
#include <sys/epoll.h>

using namespace muduo;
using namespace muduo::net;

EPollPoller::~EPollPoller()
{
}

void EPollPoller::poll(int timeoutMs, ChannelList* activeChannels)
{
}

