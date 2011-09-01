// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is a public header file, it must only include public header files.

#ifndef MUDUO_NET_PROTORPC_RPCSERVER_H
#define MUDUO_NET_PROTORPC_RPCSERVER_H

#include <muduo/net/TcpServer.h>

namespace muduo
{
namespace net
{

class Service;

class RpcServer
{
 public:
  RpcServer(EventLoop* loop,
            const InetAddress& listenAddr);

  void setThreadNum(int numThreads)
  {
    server_.setThreadNum(numThreads);
  }

  void registerService(Service*);
  void start();

 private:
  void onConnection(const TcpConnectionPtr& conn);

  // void onMessage(const TcpConnectionPtr& conn,
  //                Buffer* buf,
  //                Timestamp time);

  EventLoop* loop_;
  TcpServer server_;
  std::map<std::string, Service*> services_;
};

}
}

#endif  // MUDUO_NET_PROTORPC_RPCSERVER_H
