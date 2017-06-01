// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: zieckey (zieckey at gmail dot com)
//
// This is a public header file, it must only include public header files.

#ifndef MUDUO_NET_UDPMESSAGE_H
#define MUDUO_NET_UDPMESSAGE_H

#include <netinet/in.h>

#include <muduo/base/Types.h>
#include <muduo/net/Buffer.h>

#include <map>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

namespace muduo
{
namespace net
{

class UdpMessage : public boost::noncopyable
{
 public:
  UdpMessage(int fd, const struct sockaddr_in& addr, size_t defaultBufferSize = 1472);
  UdpMessage(int fd, size_t defaultBufferSize = 1472);
  UdpMessage(int fd, boost::shared_ptr<Buffer>& buf);

  const struct sockaddr_in& remoteAddr() const 
  { return remoteAddr_; }

  struct sockaddr_in& mutableRemoteAddr() 
  { return remoteAddr_; }

  void setRemoteAddr(const struct sockaddr_in& addr) 
  { remoteAddr_ = addr; }
  void setRemoteAddr(const struct sockaddr& addr);

  int sockfd() const { return sockfd_; }

  boost::shared_ptr<Buffer>& buffer()
  { return buffer_; }

  static bool send(const UdpMessage* msg);

  /// helper method
 public:
  const char* data() const 
  { return buffer_->peek(); }
  size_t size() const 
  { return buffer_->readableBytes(); }
  char* beginWrite()
  { return buffer_->beginWrite(); }
 private:
  int sockfd_;
  boost::shared_ptr<Buffer> buffer_;
  struct sockaddr_in remoteAddr_;
};

typedef boost::shared_ptr<UdpMessage> UdpMessagePtr;

}
}

#endif  // MUDUO_NET_TCPSERVER_H
