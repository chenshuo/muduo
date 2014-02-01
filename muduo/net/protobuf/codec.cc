// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include <muduo/net/protobuf/codec.h>

#include <muduo/base/Logging.h>
#include <muduo/net/Endian.h>
#include <muduo/net/TcpConnection.h>
#include <muduo/net/protorpc/google-inl.h>

#include <google/protobuf/message.h>
#include <zlib.h>

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

void ProtobufCodec::send(const TcpConnectionPtr& conn,
                         const ::google::protobuf::Message& message)
{
  // FIXME: serialize to TcpConnection::outputBuffer()
  muduo::net::Buffer buf;
  fillEmptyBuffer(&buf, message);
  conn->send(&buf);
}

void ProtobufCodec::fillEmptyBuffer(muduo::net::Buffer* buf,
                                    const google::protobuf::Message& message)
{
  assert(buf->readableBytes() == 0);
  // FIXME: can we move serialization & checksum to other thread?
  buf->append(tag_);

  // code copied from MessageLite::SerializeToArray() and MessageLite::SerializePartialToArray().
  GOOGLE_DCHECK(message.IsInitialized()) << InitializationErrorMessage("serialize", message);

  int byte_size = message.ByteSize();
  buf->ensureWritableBytes(byte_size + kChecksumLen);

  uint8_t* start = reinterpret_cast<uint8_t*>(buf->beginWrite());
  uint8_t* end = message.SerializeWithCachedSizesToArray(start);
  if (end - start != byte_size)
  {
    ByteSizeConsistencyError(byte_size, message.ByteSize(), static_cast<int>(end - start));
  }
  buf->hasWritten(byte_size);

  int32_t checkSum = static_cast<int32_t>(
      ::adler32(1,
        reinterpret_cast<const Bytef*>(buf->peek()),
        static_cast<int>(buf->readableBytes())));
  buf->appendInt32(checkSum);
  assert(buf->readableBytes() == tag_.size() + byte_size + kChecksumLen);
  int32_t len = sockets::hostToNetwork32(static_cast<int32_t>(buf->readableBytes()));
  buf->prepend(&len, sizeof len);
}

void ProtobufCodec::onMessage(const TcpConnectionPtr& conn,
                              Buffer* buf,
                              Timestamp receiveTime)
{
  while (buf->readableBytes() >= static_cast<uint32_t>(kMinMessageLen+kHeaderLen))
  {
    const int32_t len = buf->peekInt32();
    if (len > kMaxMessageLen || len < kMinMessageLen)
    {
      errorCallback_(conn, buf, receiveTime, kInvalidLength);
      break;
    }
    else if (buf->readableBytes() >= implicit_cast<size_t>(len+kChecksumLen))
    {
      MessagePtr message(prototype_->New());
      // FIXME: can we move deserialization & callback to other thread?
      ErrorCode errorCode = parse(buf->peek()+kHeaderLen, len, message.get());
      if (errorCode == kNoError)
      {
        // FIXME: try { } catch (...) { }
        messageCallback_(conn, message, receiveTime);
        buf->retrieve(kHeaderLen+len);
      }
      else
      {
        errorCallback_(conn, buf, receiveTime, errorCode);
        break;
      }
    }
    else
    {
      break;
    }
  }
}

namespace
{
  const string kNoErrorStr = "NoError";
  const string kInvalidLengthStr = "InvalidLength";
  const string kCheckSumErrorStr = "CheckSumError";
  const string kInvalidNameLenStr = "InvalidNameLen";
  const string kUnknownMessageTypeStr = "UnknownMessageType";
  const string kParseErrorStr = "ParseError";
  const string kUnknownErrorStr = "UnknownError";
}

const string& ProtobufCodec::errorCodeToString(ErrorCode errorCode)
{
  switch (errorCode)
  {
   case kNoError:
     return kNoErrorStr;
   case kInvalidLength:
     return kInvalidLengthStr;
   case kCheckSumError:
     return kCheckSumErrorStr;
   case kInvalidNameLen:
     return kInvalidNameLenStr;
   case kUnknownMessageType:
     return kUnknownMessageTypeStr;
   case kParseError:
     return kParseErrorStr;
   default:
     return kUnknownErrorStr;
  }
}

void ProtobufCodec::defaultErrorCallback(const TcpConnectionPtr& conn,
                                         Buffer* buf,
                                         Timestamp,
                                         ErrorCode errorCode)
{
  LOG_ERROR << "ProtobufCodec::defaultErrorCallback - " << errorCodeToString(errorCode);
  if (conn && conn->connected())
  {
    conn->shutdown();
  }
}

int32_t ProtobufCodec::asInt32(const char* buf)
{
  int32_t be32 = 0;
  ::memcpy(&be32, buf, sizeof(be32));
  return sockets::networkToHost32(be32);
}

ProtobufCodec::ErrorCode ProtobufCodec::parse(const char* buf, int len, ::google::protobuf::Message* message)
{
  ErrorCode error = kNoError;

  // check sum
  int32_t expectedCheckSum = asInt32(buf + len - kChecksumLen);
  int32_t checkSum = static_cast<int32_t>(
      ::adler32(1,
                reinterpret_cast<const Bytef*>(buf),
                static_cast<int>(len - kChecksumLen)));

  if (checkSum == expectedCheckSum)
  {
    if (memcmp(buf, tag_.data(), tag_.size()) == 0)
    {
      // parse from buffer
      const char* data = buf + tag_.size();
      int32_t dataLen = len - kChecksumLen - static_cast<int>(tag_.size());
      if (message->ParseFromArray(data, dataLen))
      {
        error = kNoError;
      }
      else
      {
        error = kParseError;
      }
    }
    else
    {
      error = kUnknownMessageType;
    }
  }
  else
  {
    error = kCheckSumError;
  }

  return error;
}

