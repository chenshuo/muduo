// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include <muduo/net/protorpc/RpcCodec.h>

#include <muduo/base/Logging.h>
#include <muduo/net/Endian.h>
#include <muduo/net/TcpConnection.h>

#include <muduo/net/protorpc/rpc.pb.h>
#include <muduo/net/protorpc/google-inl.h>

#include <boost/bind.hpp>

using namespace muduo;
using namespace muduo::net;

namespace
{
  int ProtobufVersionCheck()
  {
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    return 0;
  }
  int dummy = ProtobufVersionCheck();
}

RpcCodec::RpcCodec(const ProtobufMessageCallback& messageCb,
                   const ErrorCallback& errorCb)
    : messageCallback_(messageCb),
      codec_(&RpcMessage::default_instance(),
             "RPC0",
             boost::bind(&RpcCodec::onRpcMessage, this, _1, _2, _3),
             errorCb)
{
}

void RpcCodec::send(const TcpConnectionPtr& conn,
                    const RpcMessage& message)
{
  codec_.send(conn, message);
}

void RpcCodec::onMessage(const TcpConnectionPtr& conn,
                         Buffer* buf,
                         Timestamp receiveTime)
{
  codec_.onMessage(conn, buf, receiveTime);
}

void RpcCodec::onRpcMessage(const TcpConnectionPtr& conn,
                            const MessagePtr& message,
                            Timestamp receiveTime)
{
  messageCallback_(conn, *::muduo::down_cast<RpcMessage*>(message.get()), receiveTime);
}

void RpcCodec::fillEmptyBuffer(muduo::net::Buffer* buf, const RpcMessage& message)
{
  codec_.fillEmptyBuffer(buf, message);
}
