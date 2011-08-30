// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include <muduo/net/protorpc/RpcChannel.h>

#include <muduo/net/protorpc/service.h>
#include <muduo/net/protorpc/rpc.pb.h>

#include <google/protobuf/descriptor.h>

#include <boost/bind.hpp>

using namespace muduo;
using namespace muduo::net;

RpcChannel::RpcChannel()
  : codec_(boost::bind(&RpcChannel::onRpcMessage, this, _1, _2, _3))
{
}

RpcChannel::RpcChannel(const TcpConnectionPtr& conn)
  : codec_(boost::bind(&RpcChannel::onRpcMessage, this, _1, _2, _3)),
    conn_(conn)
{
}

RpcChannel::~RpcChannel()
{
}

  // Call the given method of the remote service.  The signature of this
  // procedure looks the same as Service::CallMethod(), but the requirements
  // are less strict in one important way:  the request and response objects
  // need not be of any specific class as long as their descriptors are
  // method->input_type() and method->output_type().
void RpcChannel::CallMethod(const ::google::protobuf::MethodDescriptor* method,
                  RpcController* controller,
                  const ::google::protobuf::Message* request,
                  ::google::protobuf::Message* response,
                  ::google::protobuf::Closure* done)
{
  RpcMessage message;
  message.set_type(REQUEST);
  int64_t id = id_.incrementAndGet();
  message.set_id(id);
  message.set_service(method->service()->full_name());
  message.set_method(method->name());
  message.set_request(request->SerializeAsString()); // FIXME: error check
  RpcCodec::send(conn_, message);

  OutstandingCall out = { response, done };
  outstandings_[id] = out;
}

void RpcChannel::onMessage(const TcpConnectionPtr& conn,
                           Buffer* buf,
                           Timestamp receiveTime)
{
  codec_.onMessage(conn, buf, receiveTime);
}

void RpcChannel::onRpcMessage(const TcpConnectionPtr& conn,
                              const RpcMessage& message,
                              Timestamp receiveTime)
{
  assert(conn == conn_);
  //printf("%s\n", message.DebugString().c_str());
  if (message.type() == RESPONSE)
  {
    int64_t id = message.id();
    assert(message.has_response());

    std::map<int64_t, OutstandingCall>::iterator it = outstandings_.find(id);
    if (it != outstandings_.end())
    {
      OutstandingCall out = it->second;
      out.response->ParseFromString(message.response());
      out.done->Run();
      // delete out.response;
      outstandings_.erase(it);
    }
  }
  else if (message.type() == REQUEST)
  {
    // FIXME: extract to a function
    if (services_)
    {
      std::map<std::string, Service*>::const_iterator it = services_->find(message.service());
      if (it != services_->end())
      {
        Service* service = it->second;
        assert(service != NULL);
        const google::protobuf::ServiceDescriptor* desc = service->GetDescriptor();
        const google::protobuf::MethodDescriptor* method
          = desc->FindMethodByName(message.method());
        if (method)
        {
          google::protobuf::Message* request = service->GetRequestPrototype(method).New();
          request->ParseFromString(message.request());
          google::protobuf::Message* response = service->GetResponsePrototype(method).New();
          int64_t id = message.id();
          service->CallMethod(method, NULL, request, response,
              NewCallback(this, &RpcChannel::doneCallback, response, id));
        }
        else
        {
          // FIXME:
        }
      }
      else
      {
        // FIXME:
      }
    }
    else
    {
      // FIXME:
    }
  }
}

void RpcChannel::doneCallback(::google::protobuf::Message* response, int64_t id)
{
  RpcMessage message;
  message.set_type(RESPONSE);
  message.set_id(id);
  message.set_response(response->SerializeAsString()); // FIXME: error check
  RpcCodec::send(conn_, message);
}

