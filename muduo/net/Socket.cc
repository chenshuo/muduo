#include <muduo/net/Socket.h>

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>

using namespace muduo;
using namespace muduo::net;

Socket::~Socket()
{
  ::close(sockfd_);
}

void Socket::setTcpNoDelay(bool on)
{
  int optval = on ? 1 : 0;
  ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY,
               &optval, sizeof optval);
}
