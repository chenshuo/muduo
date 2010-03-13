#include <muduo/net/Channel.h>

using namespace muduo;
using namespace muduo::net;

Channel::Channel(EventLoop* loop, Socket sock)
  : loop_(loop),
    sock_(sock),
    events_(0)
{
}

Channel::~Channel()
{
}

