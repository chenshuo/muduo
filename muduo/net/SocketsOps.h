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

#ifndef MUDUO_NET_SOCKETSOPS_H
#define MUDUO_NET_SOCKETSOPS_H

#include <arpa/inet.h>

namespace muduo
{
namespace net
{
namespace sockets
{

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

///
/// Creates a non-blocking socket file descriptor,
/// abort if any error.
int createNonblockingOrDie();

}
}
}

#endif  // MUDUO_NET_SOCKETSOPS_H
