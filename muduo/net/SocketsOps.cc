// Copyright 2010 Shuo Chen
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

#include <fcntl.h>
#include <sys/socket.h>

using namespace muduo;
using namespace muduo::net;

int sockets::createNonblockingOrDie()
{
  // socket
  int sockfd = ::socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

  // reuse addr
  int one = 1;
  ::setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof one);

  // non-block
  int flags = ::fcntl(sockfd, F_GETFL, 0);
  flags |= O_NONBLOCK;
  int ret = ::fcntl(sockfd, F_SETFL, flags);

  // close-on-exec
  flags = ::fcntl(sockfd, F_GETFD, 0);
  flags |= FD_CLOEXEC;
  ret = ::fcntl(sockfd, F_SETFD, flags);

  return sockfd;
}

