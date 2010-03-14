// Copyright 2010 Shuo Chen
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

using namespace muduo;
using namespace muduo::net;

TcpServer::TcpServer(EventLoop* loop, const InetAddress& listenAddr)
  : loop_(loop),
    acceptor_(new Acceptor(loop, listenAddr))
{
}

TcpServer::~TcpServer()
{
}

void TcpServer::start()
{
}

