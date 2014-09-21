// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is a public header file, it must only include public header files.

#ifndef MUDUO_NET_INETADDRESS_H
#define MUDUO_NET_INETADDRESS_H

#include <muduo/base/copyable.h>
#include <muduo/base/StringPiece.h>

#include <netinet/in.h>

namespace muduo
{
namespace net
{

///
/// Wrapper of sockaddr_in.
///
/// This is an POD interface class.
class InetAddress : public muduo::copyable
{
 public:
  /// Constructs an endpoint with given port number.
  /// Mostly used in TcpServer listening.
  explicit InetAddress(uint16_t port = 0, bool loopbackOnly = false);

  /// Constructs an endpoint with given ip and port.
  /// @c ip should be "1.2.3.4"
  InetAddress(StringArg ip, uint16_t port);

  /// Constructs an endpoint with given struct @c sockaddr_in
  /// Mostly used when accepting new connections
  InetAddress(const struct sockaddr_in& addr)
    : addr_(addr)
  { }

  string toIp() const;
  string toIpPort() const;
  uint16_t toPort() const;

  // default copy/assignment are Okay

  const struct sockaddr_in& getSockAddrInet() const { return addr_; }
  void setSockAddrInet(const struct sockaddr_in& addr) { addr_ = addr; }

  uint32_t ipNetEndian() const { return addr_.sin_addr.s_addr; }
  uint16_t portNetEndian() const { return addr_.sin_port; }

  // resolve hostname to IP address, not changing port or sin_family
  // return true on success.
  // thread safe
  static bool resolve(StringArg hostname, InetAddress* result);
  // static std::vector<InetAddress> resolveAll(const char* hostname, uint16_t port = 0);

 private:
  struct sockaddr_in addr_;
};

}
}

#endif  // MUDUO_NET_INETADDRESS_H
