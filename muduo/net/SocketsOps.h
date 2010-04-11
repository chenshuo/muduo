// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is an internal header file, you should not include this.

#ifndef MUDUO_NET_SOCKETSOPS_H
#define MUDUO_NET_SOCKETSOPS_H

#include <arpa/inet.h>

namespace muduo
{
namespace net
{
namespace sockets
{

// the inline assembler code makes type blur,
// so we disable warnings for a while.
#pragma GCC diagnostic ignored "-Wconversion"
inline uint32_t hostToNetwork32(uint32_t hostlong)
{
  return htonl(hostlong);
}

inline uint16_t hostToNetwork16(uint16_t hostshort)
{
  return htons(hostshort);
}

inline uint32_t networkToHost32(uint32_t netlong)
{
  return ntohl(netlong);
}

inline uint16_t networkToHost16(uint16_t netshort)
{
  return ntohs(netshort);
}
#pragma GCC diagnostic error "-Wconversion"

///
/// Creates a non-blocking socket file descriptor,
/// abort if any error.
int createNonblockingOrDie();

int  connect(int sockfd, const struct sockaddr_in& addr);
void bindOrDie(int sockfd, const struct sockaddr_in& addr);
void listenOrDie(int sockfd);
int  accept(int sockfd, struct sockaddr_in* addr);
void close(int sockfd);
void shutdownWrite(int sockfd);

void toHostPort(char* buf, size_t size,
                const struct sockaddr_in& addr);
void fromHostPort(const char* ip, uint16_t port,
                  struct sockaddr_in* addr);

int getSocketError(int sockfd);

struct sockaddr_in getLocalAddr(int sockfd);
struct sockaddr_in getPeerAddr(int sockfd);
bool isSelfConnect(int sockfd);

}
}
}

#endif  // MUDUO_NET_SOCKETSOPS_H
