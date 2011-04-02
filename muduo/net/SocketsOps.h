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
#include <endian.h>

namespace muduo
{
namespace net
{
namespace sockets
{

// the inline assembler code makes type blur,
// so we disable warnings for a while.
#pragma GCC diagnostic ignored "-Wconversion"
inline uint64_t hostToNetwork64(uint64_t host64)
{
  return htobe64(host64);
}

inline uint32_t hostToNetwork32(uint32_t host32)
{
  return htonl(host32);
}

inline uint16_t hostToNetwork16(uint16_t host16)
{
  return htons(host16);
}

inline uint64_t networkToHost64(uint64_t net64)
{
  return be64toh(net64);
}

inline uint32_t networkToHost32(uint32_t net32)
{
  return ntohl(net32);
}

inline uint16_t networkToHost16(uint16_t net16)
{
  return ntohs(net16);
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
