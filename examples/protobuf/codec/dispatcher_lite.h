// Copyright 2011, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef MUDUO_EXAMPLES_PROTOBUF_CODEC_DISPATCHER_LITE_H
#define MUDUO_EXAMPLES_PROTOBUF_CODEC_DISPATCHER_LITE_H

#include <muduo/net/Callbacks.h>

#include <google/protobuf/message.h>

#include <map>

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

typedef boost::shared_ptr<google::protobuf::Message> MessagePtr;

class ProtobufDispatcherLite : boost::noncopyable
{
 public:
  typedef boost::function<void (const muduo::net::TcpConnectionPtr&,
                                const MessagePtr&,
                                muduo::Timestamp)> ProtobufMessageCallback;

  // ProtobufDispatcher()
  //   : defaultCallback_(discardProtobufMessage)
  // {
  // }

  explicit ProtobufDispatcherLite(const ProtobufMessageCallback& defaultCb)
    : defaultCallback_(defaultCb)
  {
  }

  void onProtobufMessage(const muduo::net::TcpConnectionPtr& conn,
                         const MessagePtr& message,
                         muduo::Timestamp receiveTime) const
  {
    CallbackMap::const_iterator it = callbacks_.find(message->GetDescriptor());
    if (it != callbacks_.end())
    {
      it->second(conn, message, receiveTime);
    }
    else
    {
      defaultCallback_(conn, message, receiveTime);
    }
  }

  void registerMessageCallback(const google::protobuf::Descriptor* desc,
                               const ProtobufMessageCallback& callback)
  {
    callbacks_[desc] = callback;
  }

 private:
  // static void discardProtobufMessage(const muduo::net::TcpConnectionPtr&,
  //                                    const MessagePtr&,
  //                                    muduo::Timestamp);

  typedef std::map<const google::protobuf::Descriptor*, ProtobufMessageCallback> CallbackMap;
  CallbackMap callbacks_;
  ProtobufMessageCallback defaultCallback_;
};

#endif  // MUDUO_EXAMPLES_PROTOBUF_CODEC_DISPATCHER_LITE_H

