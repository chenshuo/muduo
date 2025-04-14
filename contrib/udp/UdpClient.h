// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: zieckey (zieckey at gmail dot com)
//
// This is a public header file, it must only include public header files.

#ifndef MUDUO_CONTRIB_UDPCLIENT_H
#define MUDUO_CONTRIB_UDPCLIENT_H

#include <boost/any.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <muduo/base/Timestamp.h>

#include <muduo/net/Channel.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/Buffer.h>
#include <muduo/net/Callbacks.h>

namespace mudp
{

class UdpClient;
typedef boost::shared_ptr<UdpClient> UdpClientPtr;
typedef boost::shared_ptr<muduo::net::Buffer> BufferPtr;

class UdpClient : public boost::noncopyable, 
                  public boost::enable_shared_from_this<UdpClient>
{
 public:
  typedef boost::function<void (const UdpClientPtr&,
                              BufferPtr&,
                              muduo::Timestamp)> UdpMessageCallback;
  typedef boost::function<void (const UdpClientPtr&)> UdpWriteCompleteCallback;
 public:
  UdpClient(muduo::net::EventLoop* loop,
            const muduo::net::InetAddress& serverAddr,
            const muduo::string& name);
  ~UdpClient();  // force out-line dtor, for scoped_ptr members.

  /// virtual 'connect' the remote server
  bool connect();

  /// close the socket
  void close();

  muduo::net::EventLoop* getLoop() const { return loop_; }

  // void send(string&& message); // C++11
  void send(const void* message, size_t len);
  void send(const muduo::StringPiece& message);
  // void send(Buffer&& message); // C++11
  void send(muduo::net::Buffer* message);  // this one will swap data


  void setContext(const boost::any& context)
  { context_ = context; }

  const muduo::net::InetAddress& serverAddr() const
  { return serverAddr_; }

  const boost::any& getContext() const
  { return context_; }

  boost::any* getMutableContext()
  { return &context_; }

  /// Set message callback.
  /// Not thread safe.
  void setMessageCallback(const UdpMessageCallback& cb)
  { messageCallback_ = cb; }

  /// Set write complete callback.
  /// Not thread safe.
  void setWriteCompleteCallback(const UdpWriteCompleteCallback& cb)
  { writeCompleteCallback_ = cb; }
 private:
  void closeInLoop();
  //void sendInLoop(string&& message);
  void sendInLoop(const muduo::StringPiece& message);
  void sendInLoop(const void* message, size_t len);
  void handleRead(muduo::Timestamp receiveTime);
  void handleError();
  void handleWrite();
 private:
  muduo::net::EventLoop* loop_;
  const muduo::string name_;
  muduo::net::InetAddress serverAddr_;
  UdpMessageCallback messageCallback_;
  UdpWriteCompleteCallback writeCompleteCallback_;
  boost::scoped_ptr<muduo::net::Channel> channel_;
  bool connect_; // atomic 
  boost::any context_;
  typedef std::vector<muduo::string> stringvector;
  stringvector outputMessages_;
  stringvector outputMessagesCache_; //for effective
};

}

#endif  // MUDUO_CONTRIB_UDPCLIENT_H
