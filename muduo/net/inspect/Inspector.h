// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is a public header file, it must only include public header files.

#ifndef MUDUO_NET_INSPECT_INSPECTOR_H
#define MUDUO_NET_INSPECT_INSPECTOR_H

#include <muduo/net/http/HttpServer.h>
#include <boost/noncopyable.hpp>

namespace muduo
{
namespace net
{

// A internal inspector of the running process, usually a singleton.
class Inspector : boost::noncopyable
{
 public:
  Inspector(EventLoop* loop, 
            const InetAddress& httpAddr,
            const string& name);

 private:
  void start();

  HttpServer server_;
};

}
}

#endif  // MUDUO_NET_INSPECT_INSPECTOR_H
