// Copyright 2010 Shuo Chen (chenshuo at chenshuo dot com)
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <muduo/net/SocketsOps.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h> // FIXME
#include <stdlib.h> // FIXME
#include <sys/socket.h>
#include <unistd.h>

using namespace muduo;
using namespace muduo::net;

typedef struct sockaddr SA;

int sockets::createNonblockingOrDie()
{
  // socket
  int sockfd = ::socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  // FIXME check

  // non-block
  int flags = ::fcntl(sockfd, F_GETFL, 0);
  flags |= O_NONBLOCK;
  int ret = ::fcntl(sockfd, F_SETFL, flags);
  // FIXME check

  // close-on-exec
  flags = ::fcntl(sockfd, F_GETFD, 0);
  flags |= FD_CLOEXEC;
  ret = ::fcntl(sockfd, F_SETFD, flags);
  // FIXME check

  return sockfd;
}

void sockets::bindOrDie(int sockfd, const struct sockaddr_in& addr)
{
  int ret = ::bind(sockfd, reinterpret_cast<const SA*>(&addr), sizeof addr);
  if (ret)
  {
    perror("sockets::bindOrDie");
    abort();
  }
}

void sockets::listenOrDie(int sockfd)
{
  int ret = ::listen(sockfd, SOMAXCONN);
  if (ret)
  {
    perror("sockets::listenOrDie");
    abort();
  }
}

int sockets::accept(int sockfd, struct sockaddr_in* addr)
{
  socklen_t addrlen = sizeof *addr;
  int connfd = ::accept4(sockfd, reinterpret_cast<SA*>(addr),
                         &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
  if (connfd == -1)
  {
    int savedErrno = errno;
    perror("Socket::accept");
    switch (savedErrno)
    {
      case EAGAIN:
      case ECONNABORTED:
      case EINTR:
      case EPROTO: // ???
      case EPERM:
        // expected errors
        break;
      case EBADF:
      case EFAULT:
      case EINVAL:
      case EMFILE: // per-process lmit of open file desctiptor ???
      case ENFILE:
      case ENOBUFS:
      case ENOMEM:
      case ENOTSOCK:
      case EOPNOTSUPP:
        // unexpected errors
        abort();
        break;
      default:
        // unknown errors
        abort();
        break;
    }
  }
  return connfd;
}

void sockets::close(int sockfd)
{
  ::close(sockfd);
  // FIXME EINTR
}

void sockets::toHostPort(char* buf, size_t size,
                         const struct sockaddr_in& addr)
{
  char host[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, &addr.sin_addr, host, sizeof host);
  uint16_t port = sockets::networkToHost16(addr.sin_port);
  snprintf(buf, size, "%s:%u", host, port);
}

struct sockaddr_in sockets::getLocalAddr(int sockfd)
{
  struct sockaddr_in localaddr;
  socklen_t addrlen = sizeof(localaddr);
  ::getsockname(sockfd, reinterpret_cast<SA*>(&localaddr), &addrlen);
  // FIXME check
  return localaddr;
}
