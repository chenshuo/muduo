// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: zieckey (zieckey at gmail dot com)
//

#include "UdpClient.h"

#include <muduo/base/Logging.h>
#include <muduo/net/Connector.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/SocketsOps.h>

#include <boost/bind.hpp>

#include <stdio.h>  // snprintf
#include <errno.h> 

using namespace muduo;
using namespace muduo::net;

namespace mudp
{

UdpClient::UdpClient(EventLoop* loop,
                     const InetAddress& svrAddr,
                     const string& name)
  : loop_(CHECK_NOTNULL(loop)),
    name_(name),
    serverAddr_(svrAddr),
    connect_(false)
{
  LOG_INFO << "UdpClient::UdpClient[" << name_
           << "] - remoteAddr " << serverAddr_.toIpPort();
}

UdpClient::~UdpClient()
{
  LOG_INFO << "UdpClient::~UdpClient[" << name_
           << "] - channel_ " << get_pointer(channel_);
  loop_->assertInLoopThread();
  close();
}

bool UdpClient::connect()
{
  LOG_INFO << "UdpClient::connect[" << name_ << "] - connecting to "
           << serverAddr_.toIpPort();
  assert(!connect_);
  int sockfd = ::socket(AF_INET, SOCK_DGRAM, 0);
  assert(sockfd > 0);
  if (sockfd < 0)
  {
    return false;
  }

  LOG_TRACE << "create SOCK_DGRAM fd=" << sockfd;

//#define _DO_UDP_CONNECT
#ifdef _DO_UDP_CONNECT
  const struct sockaddr_in* remoteAddr =
      sockets::sockaddr_in_cast(serverAddr_.getSockAddrInet());
  socklen_t addrLen = sizeof(*remoteAddr);
  int ret = ::connect(sockfd, sockets::sockaddr_cast(remoteAddr), addrLen);

  if (ret != 0)
  {
    int  savedErrno = errno;
    close();
    const struct sockaddr_in *paddr = remoteAddr;
    LOG_ERROR << "Failed to connect to remote IP="
        << inet_ntoa(paddr->sin_addr)
        << ", port=" << ntohs(paddr->sin_port)
        << ", errno=" << savedErrno;
    ::close(sockfd);
    return false;
  }
#endif

  channel_.reset(new Channel(getLoop(), sockfd));
  channel_->setReadCallback(
      boost::bind(&UdpClient::handleRead, this, _1));
  channel_->setWriteCallback(
      boost::bind(&UdpClient::handleWrite, this));
  channel_->setErrorCallback(
      boost::bind(&UdpClient::handleError, this));
  channel_->tie(shared_from_this());
  channel_->enableReading();
  connect_ = true;
  return true;
}

void UdpClient::close()
{
  if (loop_->isInLoopThread())
  {
    closeInLoop();
  }
  else
  {
    loop_->runInLoop(
        boost::bind(&UdpClient::closeInLoop, this));  
  }
}

void UdpClient::closeInLoop()
{
  loop_->assertInLoopThread();
  connect_ = false;
  if (channel_.get())
  {
    channel_->disableAll();
    channel_->remove();
    if (channel_->fd() >= 0)
    {
      ::close(channel_->fd());
    }
    channel_.reset();
  }
}

void UdpClient::send(const void* data, size_t len)
{
  assert(connect_);
  if (connect_)
  {
    if (loop_->isInLoopThread())
    {
      sendInLoop(data, len);
    }
    else
    {
      string message(static_cast<const char*>(data), len);
      loop_->runInLoop(
          boost::bind(&UdpClient::sendInLoop,
                      this,
                      message));
    }
  }
}

void UdpClient::send(const StringPiece& message)
{
  assert(connect_);
  if (connect_)
  {
    if (loop_->isInLoopThread())
    {
      sendInLoop(message);
    }
    else
    {
      loop_->runInLoop(
          boost::bind(&UdpClient::sendInLoop,
                      this,
                      message.as_string()));
    }
  }
}

void UdpClient::send(Buffer* buf)
{
  assert(connect_);
  if (connect_)
  {
    if (loop_->isInLoopThread())
    {
      sendInLoop(buf->peek(), buf->readableBytes());
      buf->retrieveAll();
    }
    else
    {
      loop_->runInLoop(
          boost::bind(&UdpClient::sendInLoop,
                      this,
                      buf->retrieveAllAsString()));
    }
  }
}

void UdpClient::sendInLoop(const StringPiece& message)
{
  sendInLoop(message.data(), message.size());
}

void UdpClient::sendInLoop(const void* data, size_t len)
{
  loop_->assertInLoopThread();
  ssize_t nwrote = 0;
  bool error = false;
  if (!connect_)
  {
    LOG_WARN << "disconnected, give up writing";
    return;
  }

  //send udp data directly
#ifdef _DO_UDP_CONNECT
  nwrote = ::send(channel_->fd(), data, len, 0);
#else
  const struct sockaddr_in* remoteAddr =
      sockets::sockaddr_in_cast(serverAddr_.getSockAddr());
  socklen_t addrLen = sizeof(*remoteAddr);
  nwrote = ::sendto(channel_->fd(), data, len, 0, 
              sockets::sockaddr_cast(remoteAddr), addrLen);
#endif
  if (nwrote < 0)
  {
    nwrote = 0;
    if (errno != EWOULDBLOCK)
    {
      LOG_SYSERR << "UdpClient::sendInLoop";
      if (errno == EPIPE) // FIXME: any others?
      {
        error = true;
      }
    }

    string msg(static_cast<const char*>(data), len);
    outputMessages_.push_back(msg);//FIXME if outputMessages_ goes toooooo big
    channel_->enableWriting();
  }
  else
  {
    if (writeCompleteCallback_)
    {
      writeCompleteCallback_(shared_from_this());
    }
  }
}

void UdpClient::handleRead(Timestamp receiveTime)
{
  LOG_TRACE << " UdpClient " << receiveTime.toFormattedString();
  loop_->assertInLoopThread();
  size_t initialSize = 1472; // The UDP max payload size
  BufferPtr inputBuffer(new Buffer(initialSize));
#ifdef _DO_UDP_CONNECT
  ssize_t readn = ::recv(channel_->fd(), inputBuffer->beginWrite(), inputBuffer->writableBytes(), 0);
#else
  struct sockaddr remoteAddr;
  socklen_t addrLen = sizeof(remoteAddr);
  ssize_t readn = ::recvfrom(channel_->fd(), inputBuffer->beginWrite(), 
              inputBuffer->writableBytes(), 
              0, &remoteAddr, &addrLen);
#endif
  LOG_TRACE << "recv return, readn=" << readn << " errno=" << strerror(errno);
  if (readn >= 0)
  {
    //received a UDP data package with length = 0 is OK.
    inputBuffer->hasWritten(readn);
    if (messageCallback_)
    {
      messageCallback_(shared_from_this(), inputBuffer, receiveTime);
    }
  }
  else 
  {
    handleError();
  }
}

void UdpClient::handleError()
{
  int err = sockets::getSocketError(channel_->fd());
  LOG_ERROR << "UdpClient::handleError [" << name_
        << "] - SO_ERROR = " << err << " " << strerror_tl(err);
}

void UdpClient::handleWrite()
{
  LOG_INFO << "UdpClient::handleWrite outputMessages_.size=" << outputMessages_.size();
  if (outputMessages_.empty())
  {
    return;
  }
  assert(outputMessagesCache_.empty());
  outputMessagesCache_.swap(outputMessages_);
  stringvector::iterator it (outputMessagesCache_.begin());
  stringvector::iterator ite(outputMessagesCache_.end());
  for (; it != ite; ++it)
  {
    sendInLoop(it->data(), it->size());
  }
  outputMessagesCache_.clear();
}

}
