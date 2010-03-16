// Muduo - A lightwight C++ network library for Linux
// Copyright (c) 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Muduo team nor the names of its contributors
// may be used to endorse or promote products derived from this software
// without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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
  int connfd = ::accept(sockfd, reinterpret_cast<SA*>(addr),
                        &addrlen);//, SOCK_NONBLOCK | SOCK_CLOEXEC);
  // int connfd = ::accept4(sockfd, reinterpret_cast<SA*>(addr),
  //                        &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
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
        fprintf(stderr, "errno = %d\n", savedErrno);
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

void sockets::shutdown(int sockfd)
{
  ::shutdown(sockfd, SHUT_RDWR);
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
