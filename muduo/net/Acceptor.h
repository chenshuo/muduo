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

#ifndef MUDUO_NET_ACCEPTOR_H
#define MUDUO_NET_ACCEPTOR_H

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>

#include <muduo/net/Channel.h>
#include <muduo/net/Socket.h>

namespace muduo
{
namespace net
{

class EventLoop;
class InetAddress;

///
/// Acceptor of incoming TCP connections.
///
class Acceptor : boost::noncopyable
{
 public:
  typedef boost::function<void (int fd, const InetAddress&)>
    NewConnectionCallback;

  Acceptor(EventLoop* loop, const InetAddress& listenAddr);
  // ~Acceptor();

  void setNewConnectionCallback(const NewConnectionCallback& cb)
  { newConnectionCallback_ = cb; }
  bool listenning() const { return listenning_; }
  void listen();

 private:
  void accept();

  EventLoop* loop_;
  Socket acceptSocket_;
  Channel acceptChannel_;
  NewConnectionCallback newConnectionCallback_;
  bool listenning_;
};

}
}

#endif  // MUDUO_NET_ACCEPTOR_H
