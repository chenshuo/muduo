#include <muduo/net/Socket.h>

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
//#include <sys/types.h>

using namespace muduo;
using namespace muduo::net;

void Socket::setTcpNoDelay(bool on)
{
  int optval = on ? 1 : 0;
  ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY,
               &optval, sizeof optval);
}
