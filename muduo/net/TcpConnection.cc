// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include <muduo/net/TcpConnection.h>

#include <muduo/base/Logging.h>
#include <muduo/net/Channel.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/Socket.h>
#include <muduo/net/SocketsOps.h>

#include <boost/bind.hpp>

#include <errno.h>
#include <stdio.h>

using namespace muduo;
using namespace muduo::net;

TcpConnection::TcpConnection(EventLoop* loop,
                             const string& name__,
                             int fd,
                             const InetAddress& localAddr,
                             const InetAddress& peerAddr)
  : loop_(loop),
    name_(name__),
    state_(kConnecting),
    socket_(new Socket(fd)),
    channel_(new Channel(loop, fd)),
    localAddr_(localAddr),
    peerAddr_(peerAddr)
{
  channel_->setReadCallback(
      boost::bind(&TcpConnection::handleRead, this, _1));
  channel_->setWriteCallback(
      boost::bind(&TcpConnection::handleWrite, this));
  channel_->setCloseCallback(
      boost::bind(&TcpConnection::handleClose, this));
  channel_->setErrorCallback(
      boost::bind(&TcpConnection::handleError, this));
  printf("%p %s ctor\n", this, name_.c_str());
}

TcpConnection::~TcpConnection()
{
  printf("%p %s dtor\n", this, name_.c_str());
}

void TcpConnection::send(const string& message)
{
  if (state_ == kConnected)
  {
    loop_->runInLoop(
        boost::bind(&TcpConnection::sendInLoop,
                    this,
                    message));
                    //std::forward<string>(message)));
    // FIXME: as an optimization, send message here
  }
}

void TcpConnection::sendInLoop(const string& message)
{
  loop_->assertInLoopThread();
  // FIXME: try writing here, until EWOULDBLOCK
  outputBuffer_.append(message.data(), message.size());
  if (!channel_->isWriting())
  {
    channel_->enableWriting();
  }
}

void TcpConnection::shutdown()
{
  if (state_ == kConnected)
  {
    setState(kDisconnecting);
    loop_->runInLoop(boost::bind(&TcpConnection::shutdownInLoop, this));
  }
}

void TcpConnection::shutdownInLoop()
{
  loop_->assertInLoopThread();
  if (!channel_->isWriting())
  {
    // we are not writing
    sockets::shutdownWrite(channel_->fd());
  }
}

void TcpConnection::connectEstablished()
{
  loop_->assertInLoopThread();
  assert(state_ == kConnecting);
  setState(kConnected);
  channel_->tie(shared_from_this());
  channel_->enableReading();

  connectionCallback_(shared_from_this());
}

void TcpConnection::connectDestroyed()
{
  loop_->assertInLoopThread();
  if (state_ == kConnected)
  {
    setState(kDisconnected);
    channel_->disableAll();

    connectionCallback_(shared_from_this());
  }
  loop_->removeChannel(get_pointer(channel_));
}

void TcpConnection::handleRead(Timestamp receiveTime)
{
  loop_->assertInLoopThread();
  int savedErrno;
  ssize_t n = inputBuffer_.readFd(channel_->fd(), &savedErrno);
  if (n > 0)
  {
    messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
  }
  else if (n == 0)
  {
    handleClose();
  }
  else
  {
    // check savedErrno
  }
}

void TcpConnection::handleWrite()
{
  loop_->assertInLoopThread();
  if (channel_->isWriting())
  {
    ssize_t n = ::write(channel_->fd(), outputBuffer_.peek(), outputBuffer_.readableBytes());
    if (n > 0)
    {
      outputBuffer_.retrieve(n);
      if (outputBuffer_.readableBytes() == 0)
      {
        channel_->disableWriting();
        if (state_ == kDisconnecting)
        {
          shutdownInLoop();
        }
      }
      else
      {
        LOG_TRACE << "I am going to write more data";
      }
    }
    else
    {
      LOG_SYSERR << "TcpConnection::handleWrite";
    }
  }
  else
  {
    LOG_TRACE << "Connection is down, no more writing";
  }
}

void TcpConnection::handleClose()
{
  loop_->assertInLoopThread();
  // we don't close fd, leave it to dtor, so we can find leaks easily.
  setState(kDisconnected);
  channel_->disableAll();

  TcpConnectionPtr guardThis(shared_from_this());
  connectionCallback_(guardThis);
  // must be the last line
  closeCallback_(guardThis);
}

void TcpConnection::handleError()
{
}

