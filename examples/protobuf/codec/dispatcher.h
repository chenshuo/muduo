// Copyright 2011, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef MUDUO_EXAMPLES_PROTOBUF_CODEC_DISPATCHER_H
#define MUDUO_EXAMPLES_PROTOBUF_CODEC_DISPATCHER_H

#include "muduo/base/noncopyable.h"
#include "muduo/net/Callbacks.h"

#include <google/protobuf/message.h>

#include <map>

#include <type_traits>

typedef std::shared_ptr<google::protobuf::Message> MessagePtr;

class Callback : muduo::noncopyable
{
 public:
  virtual ~Callback() = default;
  virtual void onMessage(const muduo::net::TcpConnectionPtr&,
                         const MessagePtr& message,
                         muduo::Timestamp) const = 0;
};

template <typename T>
class CallbackT : public Callback
{
  static_assert(std::is_base_of<google::protobuf::Message, T>::value,
                "T must be derived from gpb::Message.");
 public:
  typedef std::function<void (const muduo::net::TcpConnectionPtr&,
                                const std::shared_ptr<T>& message,
                                muduo::Timestamp)> ProtobufMessageTCallback;

  CallbackT(const ProtobufMessageTCallback& callback)
    : callback_(callback)
  {
  }

  void onMessage(const muduo::net::TcpConnectionPtr& conn,
                 const MessagePtr& message,
                 muduo::Timestamp receiveTime) const override
  {
    std::shared_ptr<T> concrete = muduo::down_pointer_cast<T>(message);
    assert(concrete != NULL);
    callback_(conn, concrete, receiveTime);
  }

 private:
  ProtobufMessageTCallback callback_;
};

class ProtobufDispatcher
{
 public:
  typedef std::function<void (const muduo::net::TcpConnectionPtr&,
                                const MessagePtr& message,
                                muduo::Timestamp)> ProtobufMessageCallback;

  explicit ProtobufDispatcher(const ProtobufMessageCallback& defaultCb)
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
      it->second->onMessage(conn, message, receiveTime);
    }
    else
    {
      defaultCallback_(conn, message, receiveTime);
    }
  }

  template<typename T>
  void registerMessageCallback(const typename CallbackT<T>::ProtobufMessageTCallback& callback)
  {
    std::shared_ptr<CallbackT<T> > pd(new CallbackT<T>(callback));
    callbacks_[T::descriptor()] = pd;
  }

 private:
  typedef std::map<const google::protobuf::Descriptor*, std::shared_ptr<Callback> > CallbackMap;

  CallbackMap callbacks_;
  ProtobufMessageCallback defaultCallback_;
};
#endif  // MUDUO_EXAMPLES_PROTOBUF_CODEC_DISPATCHER_H

