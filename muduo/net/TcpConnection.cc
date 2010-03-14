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

#include <muduo/net/TcpConnection.h>

#include <muduo/net/Channel.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/Socket.h>

using namespace muduo;
using namespace muduo::net;

TcpConnection::TcpConnection(const string& name,
                             EventLoop* loop,
                             int fd,
                             const InetAddress& localAddr,
                             const InetAddress& peerAddr)
  : name_(name),
    loop_(loop),
    socket_(new Socket(fd)),
    channel_(new Channel(loop, fd)),
    localAddr_(localAddr),
    peerAddr_(peerAddr)
{
  loop_->updateChannel(get_pointer(channel_));
}

