// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is a public header file, it must only include public header files.

#ifndef MUDUO_NET_PROTORPC_RPCCODEC_H
#define MUDUO_NET_PROTORPC_RPCCODEC_H

#include "muduo/base/Timestamp.h"
#include "muduo/net/protobuf/ProtobufCodecLite.h"

namespace muduo
{
namespace net
{

class Buffer;
class TcpConnection;
typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;

class RpcMessage;
typedef std::shared_ptr<RpcMessage> RpcMessagePtr;
extern const char rpctag[];// = "RPC0";

// wire format
//
// Field     Length  Content
//
// size      4-byte  N+8
// "RPC0"    4-byte
// payload   N-byte
// checksum  4-byte  adler32 of "RPC0"+payload
//

typedef ProtobufCodecLiteT<RpcMessage, rpctag> RpcCodec;

}  // namespace net
}  // namespace muduo

#endif  // MUDUO_NET_PROTORPC_RPCCODEC_H
