// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include <muduo/net/TcpConnection.h>

#include <muduo/net/Channel.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/Socket.h>
#include <muduo/net/SocketsOps.h>

#include <boost/bind.hpp>

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
    state_(kDisconnected),
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
        boost::bind(&TcpConnection::sendInLoop, this, message));
    /*
    {
      MutexLockGuard lock(mutex_);
      outputBuffer_.append(message.data(), message.size());
    }
    channel_->set_events(Channel::kReadEvent | Channel::kWriteEvent);
    loop_->runInLoop(
        boost::bind(&EventLoop::updateChannel, loop_, get_pointer(channel_)));
    */
  }
}

void TcpConnection::sendInLoop(const string& message)
{
  loop_->assertInLoopThread();
  outputBuffer_.append(message.data(), message.size());
  if ((channel_->events() & Channel::kWriteEvent) == 0)
  {
    channel_->set_events(Channel::kReadEvent | Channel::kWriteEvent);
    loop_->updateChannel(get_pointer(channel_));
  }
}

void TcpConnection::shutdown()
{
  if (state_ == kConnected)
  {
    setState(kDisconnecting);
    sockets::shutdown(channel_->fd());
    loop_->runInLoop(boost::bind(&TcpConnection::handleClose, this));
  }
}

void TcpConnection::connectEstablished()
{
  loop_->assertInLoopThread();
  setState(kConnected);
  channel_->tie(shared_from_this());
  channel_->set_events(Channel::kReadEvent);
  loop_->updateChannel(get_pointer(channel_));

  connectionCallback_(shared_from_this());
}

void TcpConnection::connectDestroyed()
{
  loop_->assertInLoopThread();
  if (state_ == kConnected)
  {
    setState(kDisconnected);
    sockets::shutdown(channel_->fd());
    channel_->set_events(Channel::kNoneEvent);
    loop_->updateChannel(get_pointer(channel_));
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

  ssize_t n = ::write(channel_->fd(), outputBuffer_.peek(), outputBuffer_.readableBytes());
  //int savedErrno = errno;
  if (n > 0)
  {
    outputBuffer_.retrieve(n);
    if (outputBuffer_.readableBytes() == 0)
    {
      channel_->set_events(Channel::kReadEvent);
      loop_->updateChannel(get_pointer(channel_));
    }
  }
}

void TcpConnection::handleClose()
{
  loop_->assertInLoopThread();
  // we don't close fd, leave it to dtor, so we can find leaks easily.
  setState(kDisconnected);
  channel_->set_events(Channel::kNoneEvent);
  loop_->updateChannel(get_pointer(channel_));

  TcpConnectionPtr guardThis(shared_from_this());
  connectionCallback_(guardThis);
  // must be the last line
  closeCallback_(guardThis);
}

void TcpConnection::handleError()
{
}

