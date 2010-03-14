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

#ifndef MUDUO_NET_INETADDRESS_H
#define MUDUO_NET_INETADDRESS_H

#include <muduo/base/copyable.h>
#include <muduo/base/Types.h>

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
  /// Mostly used in TcpServer.
  explicit InetAddress(uint16_t port);

  /// Constructs an endpoint with given host and port.
  /// @c host could either be "1.2.3.4" or "example.com"
  InetAddress(string host, uint16_t port);

  string toHostPort();

  // default copy/assignment are Okay

  const struct sockaddr_in& getSockAddrInet() const { return addr_; }
  struct sockaddr_in* getMutableSockAddrInet() { return &addr_; }

 private:
  struct sockaddr_in addr_;
};

}
}

#endif  // MUDUO_NET_INETADDRESS_H
