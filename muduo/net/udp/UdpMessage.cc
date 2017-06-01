// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: zieckey (zieckey at gmail dot com)


#include <muduo/net/udp/UdpMessage.h>
#include <muduo/net/SocketsOps.h>
#include <muduo/base/Logging.h>

namespace muduo
{
namespace net
{

UdpMessage::UdpMessage(int fd, const struct sockaddr_in& addr, size_t defaultBufferSize)
  : sockfd_(fd), buffer_(new Buffer(defaultBufferSize)), remoteAddr_(addr)
{
}

UdpMessage::UdpMessage(int fd, size_t defaultBufferSize)
  : sockfd_(fd), buffer_(new Buffer(defaultBufferSize))
{
}

UdpMessage::UdpMessage(int fd, boost::shared_ptr<Buffer>& buf)
  : sockfd_(fd), buffer_(buf)
{
}

void UdpMessage::setRemoteAddr(const struct sockaddr& addr) 
{
  const struct sockaddr_in* in = sockets::sockaddr_in_cast(&addr);
  remoteAddr_ = *in;
}

bool UdpMessage::send(const UdpMessage* msg) {
  ssize_t sentn = ::sendto(msg->sockfd(),
              msg->data(), msg->size(), 0, 
              sockets::sockaddr_cast(&msg->remoteAddr()), 
              sizeof(msg->remoteAddr()));
  if (sentn < 0) {
    //TODO FIXME
    LOG_ERROR << "sentn=" << sentn << " , dlen=" << msg->size();
    return false;
  }

  return true;
}

}
}

