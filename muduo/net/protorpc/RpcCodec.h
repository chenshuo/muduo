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

#include <muduo/base/Timestamp.h>

#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>

namespace muduo
{
namespace net
{

class Buffer;
class TcpConnection;
typedef boost::shared_ptr<TcpConnection> TcpConnectionPtr;

class RpcMessage;

class RpcCodec
{
 public:
  enum ErrorCode
  {
    kNoError = 0,
    kInvalidLength,
    kCheckSumError,
    kInvalidNameLen,
    kUnknownMessageType,
    kParseError,
  };

  typedef boost::function<void (const TcpConnectionPtr&,
                                const RpcMessage&,
                                Timestamp)> ProtobufMessageCallback;

  typedef boost::function<void (const TcpConnectionPtr&,
                                Buffer*,
                                Timestamp,
                                ErrorCode)> ErrorCallback;

  explicit RpcCodec(const ProtobufMessageCallback& messageCb)
    : messageCallback_(messageCb),
      errorCallback_(defaultErrorCallback)
  {
  }

  RpcCodec(const ProtobufMessageCallback& messageCb, const ErrorCallback& errorCb)
    : messageCallback_(messageCb),
      errorCallback_(errorCb)
  {
  }

  static void send(const TcpConnectionPtr& conn,
                   const RpcMessage& message);

  void onMessage(const TcpConnectionPtr& conn,
                 Buffer* buf,
                 Timestamp receiveTime);

  static const string& errorCodeToString(ErrorCode errorCode);
  static ErrorCode parse(const char* buf, int len, RpcMessage* message);
  static int32_t asInt32(const char* buf);

  static void defaultErrorCallback(const TcpConnectionPtr&,
                                   Buffer*,
                                   Timestamp,
                                   ErrorCode);

 private:
  ProtobufMessageCallback messageCallback_;
  ErrorCallback errorCallback_;

  const static int kHeaderLen = sizeof(int32_t);
  const static int kMinMessageLen = 2*kHeaderLen; // RPC0 + checkSum
  const static int kMaxMessageLen = 64*1024*1024; // same as codec_stream.h kDefaultTotalBytesLimit
};

}
}

#endif  // MUDUO_NET_PROTORPC_RPCCODEC_H
