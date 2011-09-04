// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include <muduo/net/protorpc2/RpcServer.h>

#include <muduo/base/Logging.h>
#include <muduo/net/protorpc2/RpcChannel.h>
#include <muduo/net/protorpc2/service.h>

#include <google/protobuf/descriptor.h>

#include <boost/bind.hpp>

using namespace muduo;
using namespace muduo::net;

RpcServer::RpcServer(EventLoop* loop,
                       const InetAddress& listenAddr)
  : loop_(loop),
    server_(loop, listenAddr, "RpcServer")
{
  server_.setConnectionCallback(
      boost::bind(&RpcServer::onConnection, this, _1));
//   server_.setMessageCallback(
//       boost::bind(&RpcServer::onMessage, this, _1, _2, _3));
}

void RpcServer::registerService(muduo::net::Service* service)
{
  const google::protobuf::ServiceDescriptor* desc = service->GetDescriptor();
  services_[desc->full_name()] = service;
}

void RpcServer::start()
{
  server_.start();
}

void RpcServer::onConnection(const TcpConnectionPtr& conn)
{
  LOG_INFO << "RpcServer - " << conn->peerAddress().toHostPort() << " -> "
    << conn->localAddress().toHostPort() << " is "
    << (conn->connected() ? "UP" : "DOWN");
  if (conn->connected())
  {
    RpcChannelPtr channel(new RpcChannel(conn));
    channel->setServices(&services_);
    conn->setMessageCallback(
        boost::bind(&RpcChannel::onMessage, get_pointer(channel), _1, _2, _3));
    conn->setContext(channel);
  }
  else
  {
    conn->setContext(RpcChannelPtr());
    // FIXME:
  }
}

// void RpcServer::onMessage(const TcpConnectionPtr& conn,
//                           Buffer* buf,
//                           Timestamp time)
// {
//   RpcChannelPtr& channel = boost::any_cast<RpcChannelPtr&>(conn->getContext());
//   channel->onMessage(conn, buf, time);
// }

