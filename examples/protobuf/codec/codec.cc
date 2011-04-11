// Copyright 2011, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "codec.h"

#include <muduo/base/Logging.h>
#include <muduo/net/SocketsOps.h>

#include <google/protobuf/descriptor.h>

#include <zlib.h>  // adler32

using namespace muduo;
using namespace muduo::net;

// ByteSizeConsistencyError and InitializationErrorMessage are
// copied from google/protobuf/message_lite.cc

// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
// http://code.google.com/p/protobuf/
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// Authors: wink@google.com (Wink Saville),
//          kenton@google.com (Kenton Varda)
//  Based on original Protocol Buffers design by
//  Sanjay Ghemawat, Jeff Dean, and others.


// When serializing, we first compute the byte size, then serialize the message.
// If serialization produces a different number of bytes than expected, we
// call this function, which crashes.  The problem could be due to a bug in the
// protobuf implementation but is more likely caused by concurrent modification
// of the message.  This function attempts to distinguish between the two and
// provide a useful error message.
void ByteSizeConsistencyError(int byte_size_before_serialization,
                              int byte_size_after_serialization,
                              int bytes_produced_by_serialization)
{
  GOOGLE_CHECK_EQ(byte_size_before_serialization, byte_size_after_serialization)
      << "Protocol message was modified concurrently during serialization.";
  GOOGLE_CHECK_EQ(bytes_produced_by_serialization, byte_size_before_serialization)
      << "Byte size calculation and serialization were inconsistent.  This "
         "may indicate a bug in protocol buffers or it may be caused by "
         "concurrent modification of the message.";
  GOOGLE_LOG(FATAL) << "This shouldn't be called if all the sizes are equal.";
}

std::string InitializationErrorMessage(const char* action,
                                       const google::protobuf::MessageLite& message)
{
  // Note:  We want to avoid depending on strutil in the lite library, otherwise
  //   we'd use:
  //
  // return strings::Substitute(
  //   "Can't $0 message of type \"$1\" because it is missing required "
  //   "fields: $2",
  //   action, message.GetTypeName(),
  //   message.InitializationErrorString());

  std::string result;
  result += "Can't ";
  result += action;
  result += " message of type \"";
  result += message.GetTypeName();
  result += "\" because it is missing required fields: ";
  result += message.InitializationErrorString();
  return result;
}

void ProtobufCodec::fillEmptyBuffer(Buffer* buf, const google::protobuf::Message& message)
{
  // buf->retrieveAll();
  assert(buf->readableBytes() == 0);

  const std::string& typeName = message.GetTypeName();
  int32_t nameLen = static_cast<int32_t>(typeName.size()+1);
  buf->appendInt32(nameLen);
  buf->append(typeName.c_str(), nameLen);

  // code copied from MessageLite::SerializeToArray() and MessageLite::SerializePartialToArray().
  GOOGLE_DCHECK(message.IsInitialized()) << InitializationErrorMessage("serialize", message);

  int byte_size = message.ByteSize();
  buf->ensureWritableBytes(byte_size);

  uint8_t* start = reinterpret_cast<uint8_t*>(buf->beginWrite());
  uint8_t* end = message.SerializeWithCachedSizesToArray(start);
  if (end - start != byte_size)
  {
    ByteSizeConsistencyError(byte_size, message.ByteSize(), static_cast<int>(end - start));
  }
  buf->hasWritten(byte_size);

  int32_t checkSum = static_cast<int32_t>(
      ::adler32(0,
                reinterpret_cast<const Bytef*>(buf->peek()),
                static_cast<int>(buf->readableBytes())));
  buf->appendInt32(checkSum);
  assert(buf->readableBytes() == sizeof nameLen + nameLen + byte_size + sizeof checkSum);
  int32_t len = sockets::hostToNetwork32(static_cast<int32_t>(buf->readableBytes()));
  buf->prepend(&len, sizeof len);
}

//
// no more google code after this
//

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

void ProtobufCodec::defaultErrorCallback(const muduo::net::TcpConnectionPtr& conn,
                                         muduo::net::Buffer* buf,
                                         muduo::Timestamp,
                                         ErrorCode errorCode)
{
  LOG_ERROR << "ProtobufCodec::defaultErrorCallback - " << errorCodeToString(errorCode);
  if (conn && conn->connected())
  {
    conn->shutdown();
  }
}

int32_t asInt32(const char* buf)
{
  int32_t be32 = 0;
  ::memcpy(&be32, buf, sizeof(be32));
  return sockets::networkToHost32(be32);
}

void ProtobufCodec::onMessage(const TcpConnectionPtr& conn,
                              Buffer* buf,
                              Timestamp receiveTime)
{
  while (buf->readableBytes() >= kMinMessageLen + kHeaderLen)
  {
    const int32_t len = buf->peekInt32();
    if (len > kMaxMessageLen || len < kMinMessageLen)
    {
      errorCallback_(conn, buf, receiveTime, kInvalidLength);
    }
    else if (buf->readableBytes() >= implicit_cast<size_t>(len + kHeaderLen))
    {
      ErrorCode errorCode = kNoError;
      MessagePtr message = parse(buf->peek()+kHeaderLen, len, &errorCode);
      if (errorCode == kNoError && message)
      {
        messageCallback_(conn, message, receiveTime);
        buf->retrieve(kHeaderLen+len);
      }
      else
      {
        errorCallback_(conn, buf, receiveTime, errorCode);
      }
    }
    else
    {
      break;
    }
  }
}

google::protobuf::Message* ProtobufCodec::createMessage(const std::string& typeName)
{
  google::protobuf::Message* message = NULL;
  const google::protobuf::Descriptor* descriptor =
    google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(typeName);
  if (descriptor)
  {
    const google::protobuf::Message* prototype =
      google::protobuf::MessageFactory::generated_factory()->GetPrototype(descriptor);
    if (prototype)
    {
      message = prototype->New();
    }
  }
  return message;
}

MessagePtr ProtobufCodec::parse(const char* buf, int len, ErrorCode* error)
{
  MessagePtr message;

  // check sum
  int32_t expectedCheckSum = asInt32(buf + len - kHeaderLen);
  int32_t checkSum = static_cast<int32_t>(
      ::adler32(0,
                reinterpret_cast<const Bytef*>(buf),
                static_cast<int>(len - kHeaderLen)));
  if (checkSum == expectedCheckSum)
  {
    // get message type name
    int32_t nameLen = asInt32(buf);
    if (nameLen >= 2 && nameLen <= len - 2*kHeaderLen)
    {
      std::string typeName(buf + kHeaderLen, buf + kHeaderLen + nameLen - 1);
      // create message object
      message.reset(createMessage(typeName));
      if (message)
      {
        // parse from buffer
        const char* data = buf + kHeaderLen + nameLen;
        int32_t dataLen = len - nameLen - 2*kHeaderLen;
        if (message->ParseFromArray(data, dataLen))
        {
          *error = kNoError;
        }
        else
        {
          *error = kParseError;
        }
      }
      else
      {
        *error = kUnknownMessageType;
      }
    }
    else
    {
      *error = kInvalidNameLen;
    }
  }
  else
  {
    *error = kCheckSumError;
  }

  return message;
}
