// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is a public header file, it must only include public header files.

#ifndef MUDUO_NET_PROTORPC_SERVICE_H
#define MUDUO_NET_PROTORPC_SERVICE_H

#include <boost/noncopyable.hpp>
#include <muduo/net/protorpc2/RpcChannel.h>

// Service and RpcChannel classes are incorporated from
// google/protobuf/service.h

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

// Author: kenton@google.com (Kenton Varda)
//  Based on original Protocol Buffers design by
//  Sanjay Ghemawat, Jeff Dean, and others.

namespace google {
namespace protobuf {

// Defined in other files.
class Descriptor;            // descriptor.h
class ServiceDescriptor;     // descriptor.h
class MethodDescriptor;      // descriptor.h
class Message;               // message.h

class Closure;

}  // namespace protobuf
}  // namespace google


namespace muduo
{
namespace net
{

class RpcChannel;
// Defined in this file.
class Service;
class RpcController;

// Abstract base interface for protocol-buffer-based RPC services.  Services
// themselves are abstract interfaces (implemented either by servers or as
// stubs), but they subclass this base interface.  The methods of this
// interface can be used to call the methods of the Service without knowing
// its exact type at compile time (analogous to Reflection).
class Service : boost::noncopyable
{
 public:
  Service() {}
  virtual ~Service() {}

  // Get the ServiceDescriptor describing this service and its methods.
  virtual const ::google::protobuf::ServiceDescriptor* GetDescriptor() = 0;

  // Call a method of the service specified by MethodDescriptor.  This is
  // normally implemented as a simple switch() that calls the standard
  // definitions of the service's methods.
  //
  // Preconditions:
  // * method->service() == GetDescriptor()
  // * request and response are of the exact same classes as the objects
  //   returned by GetRequestPrototype(method) and
  //   GetResponsePrototype(method).
  // * After the call has started, the request must not be modified and the
  //   response must not be accessed at all until "done" is called.
  // * "controller" is of the correct type for the RPC implementation being
  //   used by this Service.  For stubs, the "correct type" depends on the
  //   RpcChannel which the stub is using.  Server-side Service
  //   implementations are expected to accept whatever type of RpcController
  //   the server-side RPC implementation uses.
  //
  // Postconditions:
  // * "done" will be called when the method is complete.  This may be
  //   before CallMethod() returns or it may be at some point in the future.
  // * If the RPC succeeded, "response" contains the response returned by
  //   the server.
  // * If the RPC failed, "response"'s contents are undefined.  The
  //   RpcController can be queried to determine if an error occurred and
  //   possibly to get more information about the error.
  virtual void CallMethod(const ::google::protobuf::MethodDescriptor* method,
                          RpcController* controller,
                          const ::google::protobuf::Message* request,
                          ::google::protobuf::Message* response,
                          ::google::protobuf::Closure* done) = 0;

  // CallMethod() requires that the request and response passed in are of a
  // particular subclass of Message.  GetRequestPrototype() and
  // GetResponsePrototype() get the default instances of these required types.
  // You can then call Message::New() on these instances to construct mutable
  // objects which you can then pass to CallMethod().
  //
  // Example:
  //   const MethodDescriptor* method =
  //     service->GetDescriptor()->FindMethodByName("Foo");
  //   Message* request  = stub->GetRequestPrototype (method)->New();
  //   Message* response = stub->GetResponsePrototype(method)->New();
  //   request->ParseFromString(input);
  //   service->CallMethod(method, *request, response, callback);
  virtual const ::google::protobuf::Message& GetRequestPrototype(
    const ::google::protobuf::MethodDescriptor* method) const = 0;
  virtual const ::google::protobuf::Message& GetResponsePrototype(
    const ::google::protobuf::MethodDescriptor* method) const = 0;

};

}
}

#endif  // MUDUO_NET_PROTORPC_SERVICE_H
