// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//

#include <muduo/net/inspect/Inspector.h>
#include <muduo/net/EventLoop.h>

#include <boost/bind.hpp>

using namespace muduo;
using namespace muduo::net;

Inspector::Inspector(EventLoop* loop, 
            const InetAddress& httpAddr,
            const string& name)
    : server_(loop, httpAddr, "Inspector:"+name)
{
  loop->runAfter(0, boost::bind(&Inspector::start, this));
}

void Inspector::start()
{
  server_.start();
}
