// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is a public header file, it must only include public header files.

#ifndef MUDUO_NET_TCPCLIENT_H
#define MUDUO_NET_TCPCLIENT_H

#include <boost/noncopyable.hpp>

#include <muduo/net/TcpConnection.h>

namespace muduo
{
namespace net
{

class Connector;
class EventLoop;

class TcpClient : boost::noncopyable
{
 public:
  TcpClient(EventLoop* loop, const string& host, uint16_t port);
  TcpClient(EventLoop* loop, const InetAddress& serverAddr);
  ~TcpClient();  // force out-line dtor, for scoped_ptr members.

  void connect();
  void disconnect();

  /// Set connection callback.
  /// Not thread safe.
  void setConnectionCallback(const ConnectionCallback& cb)
  { connectionCallback_ = cb; }

  /// Set message callback.
  /// Not thread safe.
  void setMessageCallback(const MessageCallback& cb)
  { messageCallback_ = cb; }

 private:
  EventLoop* loop_;
  InetAddress serverAddr_;
  boost::scoped_ptr<Connector> connector_; // avoid revealing Connector
  ConnectionCallback connectionCallback_;
  MessageCallback messageCallback_;
  TcpConnectionPtr connection_;
};

}
}

#endif  // MUDUO_NET_TCPCLIENT_H
