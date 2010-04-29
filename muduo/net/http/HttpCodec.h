// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is an internal header file, you should not include this.

#ifndef MUDUO_NET_HTTP_HTTPCODEC_H
#define MUDUO_NET_HTTP_HTTPCODEC_H

#include <muduo/net/TcpConnection.h>

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>

namespace muduo
{
namespace net
{

class HttpRequest;

class HttpCodec : boost::noncopyable
{
 public:
  typedef boost::function<void (const TcpConnectionPtr&,
                                const HttpRequest&)> HttpRequestCallback;

  explicit HttpCodec(const HttpRequestCallback& cb)
    : requestCallback_(cb)
  {
  }

  void onMessage(const TcpConnectionPtr& conn,
                 Buffer* buf,
                 Timestamp receiveTime);

 private:
  HttpRequestCallback requestCallback_;
};

}
}

#endif  // MUDUO_NET_HTTP_HTTPCODEC_H
