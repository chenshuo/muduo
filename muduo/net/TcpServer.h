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

#ifndef MUDUO_NET_TCPSERVER_H
#define MUDUO_NET_TCPSERVER_H

#include <muduo/net/Callbacks.h>

#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

namespace muduo
{
namespace net
{

class Acceptor;
class EventLoop;
class InetAddress;
class ThreadModel;

///
/// TCP server, supports single-threaded and thread-pool models.
///
/// This is an interface class, so don't expose too much details.
class TcpServer : boost::noncopyable
{
 public:

  TcpServer(EventLoop* loop, const InetAddress& listenAddr);
  ~TcpServer();  // force out-line dtor, for scoped_ptr members.

  /// Set the number of threads for handling input.
  ///
  /// Always accepts new connection in loop's thread.
  /// Must be called before @c start
  /// @param numThreads
  /// - 0 means all I/O in loop's thread, no thread will created.
  ///   this is the default value.
  /// - 1 means all I/O in another thread.
  /// - N means a thread pool with N threads, new connections
  ///   are assigned on a round-robin basis.
  void setThreadNum(int numThreads);

  /// Starts the server if it's not listenning.
  ///
  /// It's harmless to call it multiple times.
  /// Not thread safe.
  void start();

  /// Set connection callback.
  /// Not thread safe.
  void setConnectionCallback(const ConnectionCallback& cb)
  { connectionCallback_ = cb; }

  /// Set message callback.
  /// Not thread safe.
  void setMessageCallback(const MessageCallback& cb)
  { messageCallback_ = cb; }

 private:
  /// Not thread safe.
  void newConnection(int fd, const InetAddress& peerAddr);

  EventLoop* loop_;  // the acceptor loop
  boost::scoped_ptr<Acceptor> acceptor_; // avoid revealing Acceptor
  boost::scoped_ptr<ThreadModel> threadModel_;
  ConnectionCallback connectionCallback_;
  MessageCallback messageCallback_;
  bool started_;
};

}
}

#endif  // MUDUO_NET_TCPSERVER_H
