// Muduo - A lightwight C++ network library for Linux
// Copyright (c) 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Muduo team nor the names of its contributors
// may be used to endorse or promote products derived from this software
// without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is a public header file, it must only include public header files.

#ifndef MUDUO_NET_TCPCONNECTION_H
#define MUDUO_NET_TCPCONNECTION_H

#include <muduo/base/Mutex.h>
#include <muduo/base/Types.h>
#include <muduo/net/Callbacks.h>
#include <muduo/net/ChannelBuffer.h>
#include <muduo/net/InetAddress.h>

#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

namespace muduo
{
namespace net
{

class Channel;
class EventLoop;
class Socket;

///
/// TCP connection, for both client and server usage.
///
/// This is an interface class, so don't expose too much details.
class TcpConnection : public boost::enable_shared_from_this<TcpConnection>,
                      boost::noncopyable
{
 public:
  /// Constructs a TcpConnection with a connected sockfd
  ///
  TcpConnection(EventLoop* loop,
                const string& name,
                int sockfd,
                const InetAddress& localAddr,
                const InetAddress& peerAddr);
  ~TcpConnection();

  EventLoop* getLoop() const { return loop_; }
  const string& name() const { return name_; }
  const InetAddress& localAddress() { return localAddr_; }
  const InetAddress& peerAddress() { return peerAddr_; }
  bool connected() const { return state_ == kConnected; }
  bool connecting() const { return state_ == kConnecting; }

  void send(const string& message);
  // void send(const ChannelBuffer& message);
  void send(ChannelBuffer* message);  // this one will retrieve data
  void shutdown();

  void setConnectionCallback(ConnectionCallback cb)
  { connectionCallback_ = cb; }

  void setMessageCallback(MessageCallback cb)
  { messageCallback_ = cb; }

  /// Internal use only.
  void setCloseCallback(ConnectionCallback cb)
  { closeCallback_ = cb; }

  // called when TcpServer accepts a new connection
  void connectEstablished();
  void connectDestroyed();

 private:
  enum States { kDisconnected, kConnecting, kConnected, kDisconnecting };
  void handleRead(Timestamp receiveTime);
  void handleWrite();
  void handleClose();
  void handleError();
  void sendInLoop(const string& message);

  EventLoop* loop_;
  string name_;
  States state_;
  // we don't expose those classes to client.
  boost::scoped_ptr<Socket> socket_;
  boost::scoped_ptr<Channel> channel_;
  InetAddress localAddr_;
  InetAddress peerAddr_;
  ConnectionCallback connectionCallback_;
  MessageCallback messageCallback_;
  ConnectionCallback closeCallback_;
  ChannelBuffer inputBuffer_;
  // MutexLock mutex_;
  ChannelBuffer outputBuffer_;
};

typedef boost::shared_ptr<TcpConnection> TcpConnectionPtr;

}
}

#endif  // MUDUO_NET_TCPCONNECTION_H
