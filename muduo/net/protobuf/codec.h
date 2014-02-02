// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is a public header file, it must only include public header files.

// For Protobuf codec supporting multiple message types, check
// examples/protobuf/codec

#ifndef MUDUO_NET_PROTOBUF_CODEC_H
#define MUDUO_NET_PROTOBUF_CODEC_H

#include <muduo/base/StringPiece.h>
#include <muduo/base/Timestamp.h>

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

namespace google
{
namespace protobuf
{
class Message;
}
}

namespace muduo
{
namespace net
{

class Buffer;
class TcpConnection;
typedef boost::shared_ptr<TcpConnection> TcpConnectionPtr;
typedef boost::shared_ptr<google::protobuf::Message> MessagePtr;

template<typename MSG>
class ProtobufCodecT
{
 public:
  typedef boost::shared_ptr<MSG> MessagePtr;
  typedef boost::function<void (const TcpConnectionPtr&,
                                const MessagePtr&,
                                Timestamp)> ProtobufMessageCallback;
};

// wire format
//
// Field     Length  Content
//
// size      4-byte  M+N+4
// tag       M-byte  could be "RPC0", etc.
// payload   N-byte
// checksum  4-byte  adler32 of tag+payload
//
// This is an internal class, you should use ProtobufCodecT instead.
class ProtobufCodec : boost::noncopyable
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
                                const MessagePtr&,
                                Timestamp)> ProtobufMessageCallback;

  typedef boost::function<void (const TcpConnectionPtr&,
                                Buffer*,
                                Timestamp,
                                ErrorCode)> ErrorCallback;

  ProtobufCodec(const ::google::protobuf::Message* prototype,
                StringPiece tag,
                const ProtobufMessageCallback& messageCb,
                const ErrorCallback& errorCb = defaultErrorCallback)
    : prototype_(prototype),
      tag_(tag.as_string()),
      messageCallback_(messageCb),
      errorCallback_(errorCb),
      kMinMessageLen(tag.size() + kChecksumLen)
  {
  }

  void send(const TcpConnectionPtr& conn,
            const ::google::protobuf::Message& message);

  void onMessage(const TcpConnectionPtr& conn,
                 Buffer* buf,
                 Timestamp receiveTime);

  static const string& errorCodeToString(ErrorCode errorCode);

  // public for unit tests
  ErrorCode parse(const char* buf, int len, ::google::protobuf::Message* message);
  void fillEmptyBuffer(muduo::net::Buffer* buf, const google::protobuf::Message& message);

  static int32_t asInt32(const char* buf);
  static void defaultErrorCallback(const TcpConnectionPtr&,
                                   Buffer*,
                                   Timestamp,
                                   ErrorCode);

 private:
  const ::google::protobuf::Message* prototype_;
  const string tag_;
  ProtobufMessageCallback messageCallback_;
  ErrorCallback errorCallback_;
  const int kMinMessageLen;

  const static int kHeaderLen = sizeof(int32_t);
  const static int kChecksumLen = sizeof(int32_t);
  const static int kMaxMessageLen = 64*1024*1024; // same as codec_stream.h kDefaultTotalBytesLimit
};

}
}

#endif  // MUDUO_NET_PROTOBUF_CODEC_H
