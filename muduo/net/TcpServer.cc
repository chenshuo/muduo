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

#include <muduo/net/TcpServer.h>

#include <muduo/net/Acceptor.h>
#include <muduo/net/ThreadModel.h>

#include <boost/bind.hpp>

using namespace muduo;
using namespace muduo::net;

TcpServer::TcpServer(EventLoop* loop, const InetAddress& listenAddr)
  : loop_(loop),
    acceptor_(new Acceptor(loop, listenAddr)),
    threadModel_(new ThreadModel(loop)),
    started_(false)
{
  acceptor_->setNewConnectionCallback(
      boost::bind(&TcpServer::newConnection, this, _1, _2));
}

TcpServer::~TcpServer()
{
}

void TcpServer::setThreadNum(int numThreads)
{
  assert(0 <= numThreads);
  threadModel_->setThreadNum(numThreads);
}

void TcpServer::start()
{
  if (!started_)
  {
    started_ = true;
    threadModel_->start();
  }

  if (!acceptor_->listenning())
  {
    acceptor_->listen();
  }
}

void TcpServer::newConnection(int fd, const InetAddress& peerAddr)
{

}
