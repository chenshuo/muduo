// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: zieckey (zieckey at gmail dot com)
//
// This is a public header file, it must only include public header files.

#ifndef MUDUO_CONTRIB_UDPSERVER_H
#define MUDUO_CONTRIB_UDPSERVER_H

#include <muduo/base/Types.h>

#include <boost/any.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <muduo/base/Timestamp.h>

#include <muduo/net/Callbacks.h>
#include <muduo/net/Channel.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/EventLoopThreadPool.h>

#include "UdpMessage.h"

namespace mudp
{

using namespace muduo;
using namespace muduo::net;

class UdpServer;
typedef boost::shared_ptr<UdpServer> UdpServerPtr;

///
/// UDP server, supports single-threaded and thread-pool models.
///
/// This is an interface class, so don't expose too much details.
class UdpServer : public boost::noncopyable, 
                  public boost::enable_shared_from_this<UdpServer>
{
 public:
  typedef boost::function<void (muduo::net::EventLoop* loop,
                                const UdpServerPtr&, 
                                UdpMessagePtr&,
                                muduo::Timestamp)> UdpMessageCallback;
  typedef boost::function<void(muduo::net::EventLoop*)> ThreadInitCallback;

  //UdpServer(EventLoop* loop, const InetAddress& listenAddr);
  UdpServer(muduo::net::EventLoop* loop,
            const muduo::net::InetAddress& listenAddr,
            const muduo::string& nameArg);
  ~UdpServer();  // force out-line dtor, for scoped_ptr members.

  const muduo::string& hostport() const { return hostport_; }
  const muduo::string& name() const { return name_; }
  muduo::net::EventLoop* getLoop() const { return loop_; }

  /// Set the number of threads for handling input.
  ///
  /// Always receives new messages in loop's thread.
  /// Must be called before @c start
  /// @param numThreads
  /// - 0 means all I/O in loop's thread, no thread will created.
  ///   this is the default value.
  /// - 1 means all I/O in another thread.
  /// - N means a thread pool with N threads, new messages
  ///   are dipatched on a round-robin basis.
  void setThreadNum(int numThreads);
  void setThreadInitCallback(const ThreadInitCallback& cb)
  { threadInitCallback_ = cb; }
  void setThreadPool(boost::shared_ptr<muduo::net::EventLoopThreadPool> pool)
  { threadPool_ = pool; }

  /// Starts the server if it's not listenning.
  ///
  /// It's harmless to call it multiple times.
  /// Thread safe.
  void start();

  void stop();

  /// Set message callback.
  /// Not thread safe.
  void setMessageCallback(const UdpMessageCallback& cb)
  { messageCallback_ = cb; }

  /// Set write complete callback.
  /// Not thread safe.
  void setWriteCompleteCallback(const WriteCompleteCallback& cb)
  { writeCompleteCallback_ = cb; }

  boost::shared_ptr<muduo::net::EventLoopThreadPool> threadPool() const 
  { return threadPool_; }

  void setContext(const boost::any& context)
  { context_ = context; }

  const boost::any& getContext() const
  { return context_; }

  boost::any* getMutableContext()
  { return &context_; }


 private:
  void stopInLoop();
  void handleRead(muduo::Timestamp receiveTime);
  void handleError();
  void handleWrite();
  EventLoop* getNextLoop(const UdpMessagePtr& msg);
 private:
  muduo::net::EventLoop* loop_;  // the receiving loop
  muduo::net::InetAddress listenAddr_;
  const muduo::string hostport_;
  const muduo::string name_;
  boost::shared_ptr<EventLoopThreadPool> threadPool_;
  UdpMessageCallback messageCallback_;
  WriteCompleteCallback writeCompleteCallback_;
  ThreadInitCallback threadInitCallback_;
  bool started_;
  boost::scoped_ptr<muduo::net::Channel> channel_;
  boost::any context_;
};

}

#endif  // MUDUO_CONTRIB_UDPSERVER_H
