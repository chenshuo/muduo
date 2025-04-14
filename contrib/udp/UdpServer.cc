// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: zieckey (zieckey at gmail dot com)

#include "UdpServer.h"

#include <stdio.h>
#include <errno.h> 

#include <boost/bind.hpp>

#include <muduo/base/Logging.h>
#include <muduo/net/Acceptor.h>
#include <muduo/net/SocketsOps.h>

using namespace muduo;
using namespace muduo::net;

namespace mudp
{

UdpServer::UdpServer(EventLoop* loop,
                     const InetAddress& listenAddr,
                     const string& nameArg)
  : loop_(CHECK_NOTNULL(loop)),
    listenAddr_(listenAddr),
    hostport_(listenAddr.toIpPort()),
    name_(nameArg),
    threadPool_(new EventLoopThreadPool(loop, "UdpServer")),
    started_(false)
{
}

UdpServer::~UdpServer()
{
  loop_->assertInLoopThread();
  LOG_TRACE << "UdpServer::~UdpServer [" << name_ << "] destructing";
  stop();
}

void UdpServer::setThreadNum(int numThreads)
{
  assert(0 <= numThreads);
  threadPool_->setThreadNum(numThreads);
}

void UdpServer::start()
{
  if (started_)
  {
    return;
  }

  if (!threadPool_->started())
  {
    threadPool_->start(threadInitCallback_);
  }

  int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  assert(sockfd > 0);
  if (sockfd < 0)
  {
    LOG_SYSFATAL << "socket(AF_INET, SOCK_DGRAM, 0) failed!";
    return;
  }
  LOG_TRACE << "create SOCK_DGRAM fd=" << sockfd;
  sockets::bindOrDie(sockfd, listenAddr_.getSockAddr());

  int optval = 1;
  int opret = ::setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,
              &optval, static_cast<socklen_t>(sizeof optval));
  if (opret < 0)
  {
    LOG_SYSFATAL << "setsockopt SO_REUSEADDR failed!";
  }

  channel_.reset(new Channel(getLoop(), sockfd));
  channel_->setReadCallback(
      boost::bind(&UdpServer::handleRead, this, _1));
  channel_->setWriteCallback(
      boost::bind(&UdpServer::handleWrite, this));
  channel_->setErrorCallback(
      boost::bind(&UdpServer::handleError, this));
  channel_->tie(shared_from_this());
  channel_->enableReading();


  started_ = true;
}

void UdpServer::stop()
{
  if (loop_->isInLoopThread())
  {
    stopInLoop();
  }
  else
  {
    loop_->runInLoop(
        boost::bind(&UdpServer::stopInLoop, this));  
  }
}

void UdpServer::stopInLoop()
{
  loop_->assertInLoopThread();
  if (channel_.get())
  {
    if (channel_->fd() > 0)
    {
      ::close(channel_->fd());
    }
    channel_->disableAll();
    channel_->remove();
    channel_.reset();
  }
}

void UdpServer::handleRead(Timestamp receiveTime)
{
  LOG_TRACE << " UdpServer " << receiveTime.toFormattedString();
  loop_->assertInLoopThread();
  const size_t initialSize = 1472; // The UDP max payload size
  for (;;) 
  {
    UdpMessagePtr msg(new UdpMessage(channel_->fd(), initialSize));
    boost::shared_ptr<Buffer>& inputBuffer = msg->buffer();
    struct sockaddr remoteAddr;
    socklen_t addrLen = sizeof(remoteAddr);
    ssize_t readn = ::recvfrom(channel_->fd(), inputBuffer->beginWrite(),
                inputBuffer->writableBytes(), 0, &remoteAddr, &addrLen);
    LOG_TRACE << "recv return, readn=" << readn
        << " errno=" << strerror(errno) << " "
        << InetAddress(*sockets::sockaddr_in_cast(&remoteAddr)).toIpPort();
    if (readn >= 0)
    {
      //received a UDP data package with length = 0 is OK.
      inputBuffer->hasWritten(readn);
      msg->setRemoteAddr(remoteAddr);
      if (messageCallback_) 
      {
        // Get an EventLoop associated with a worker thread
        EventLoop* loop = getNextLoop(msg);
        loop->runInLoop(boost::bind(messageCallback_, loop, shared_from_this(), 
                msg, receiveTime));
      }
    }
    else
    {
      handleError();
      break;
    }
  }
}

void UdpServer::handleError()
{
  int savedErrno = errno;
  if (savedErrno == EAGAIN || savedErrno == EWOULDBLOCK)
  {
    LOG_DEBUG << "UdpServer::handleError [" << name_
        << "] - EAGAIN or EWOULDBLOCK ERROR, errno=" << savedErrno << " " << strerror_tl(savedErrno);
  }
  else
  {
    int err = sockets::getSocketError(channel_->fd());
    LOG_ERROR << "UdpServer::handleError [" << name_
        << "] - SO_ERROR=" << err << " " << strerror_tl(err) 
        << " errno=" << savedErrno << " " << strerror_tl(savedErrno);;
  }
}

void UdpServer::handleWrite()
{
  //TODO FIXME Add implement if really necessarily
  LOG_ERROR << "UdpServer::handleWrite NOT implement";
#if 0
  LOG_INFO << "UdpServer::handleWrite outputMessages_.size=" << outputMessages_.size();
  if (outputMessages_.empty()) {
    return;
  }
  assert(outputMessagesCache_.empty());
  outputMessagesCache_.swap(outputMessages_);
  stringvector::iterator it (outputMessagesCache_.begin());
  stringvector::iterator ite(outputMessagesCache_.end());
  for (; it != ite; ++it) {
    sendInLoop(it->data(), it->size());
  }
  outputMessagesCache_.clear();
#endif
}


EventLoop* UdpServer::getNextLoop(const UdpMessagePtr& msg)
{
  const struct sockaddr_in& addr = msg->remoteAddr();
  uint64_t hash = addr.sin_port;
  hash = (hash << 32) + addr.sin_addr.s_addr;
  return threadPool_->getLoopForHash(hash);
}

}
