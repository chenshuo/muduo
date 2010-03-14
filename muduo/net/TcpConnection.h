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

#ifndef MUDUO_NET_TCPCONNECTION_H
#define MUDUO_NET_TCPCONNECTION_H

#include <muduo/base/Types.h>
#include <muduo/net/Callbacks.h>
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
  TcpConnection(const string& name,
                EventLoop* loop,
                int sockfd,
                const InetAddress& localAddr,
                const InetAddress& peerAddr);

  const InetAddress& localAddr() { return localAddr_; }
  const InetAddress& peerAddr() { return peerAddr_; }

  void setConnectionCallback(ConnectionCallback cb)
  { connectionCallback_ = cb; }

  void setMessageCallback(MessageCallback cb)
  { messageCallback_ = cb; }

  /// Internal use only.
  void setCloseCallback(ConnectionCallback cb)
  { closeCallback_ = cb; }

 private:
  string name_;
  EventLoop* loop_;
  // we don't expose those classes to client.
  boost::scoped_ptr<Socket> socket_;
  boost::scoped_ptr<Channel> channel_;
  InetAddress localAddr_;
  InetAddress peerAddr_;
  ConnectionCallback connectionCallback_;
  MessageCallback messageCallback_;
  ConnectionCallback closeCallback_;
};

typedef boost::shared_ptr<TcpConnection> TcpConnectionPtr;

}
}

#endif  // MUDUO_NET_TCPCONNECTION_H
