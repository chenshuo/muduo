// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is a public header file, it must only include public header files.

#ifndef MUDUO_NET_ENDIAN_H
#define MUDUO_NET_ENDIAN_H

#include <stdint.h>
#include <endian.h>
#include <byteswap.h>

namespace muduo
{
namespace net
{
namespace sockets
{

// the inline assembler code makes type blur,
// so we disable warnings for a while.
#if __BYTE_ORDER == __LITTLE_ENDIAN
inline uint64_t hostToNetwork64(uint64_t host64)
{
  return bswap_64(host64);
}

inline uint32_t hostToNetwork32(uint32_t host32)
{
  return bswap_32(host32);
}

inline uint16_t hostToNetwork16(uint16_t host16)
{
  return bswap_16(host16);
}

inline uint64_t networkToHost64(uint64_t net64)
{
  return bswap_64(net64);
}

inline uint32_t networkToHost32(uint32_t net32)
{
  return bswap_32(net32);
}

inline uint16_t networkToHost16(uint16_t net16)
{
  return bswap_16(net16);
}
#endif

}
}
}

#endif  // MUDUO_NET_ENDIAN_H
