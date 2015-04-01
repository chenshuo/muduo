// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is an internal header file, you should not include this.

#ifndef MUDUO_NET_INSPECT_PERFORMANCEINSPECTOR_H
#define MUDUO_NET_INSPECT_PERFORMANCEINSPECTOR_H

#include <muduo/net/inspect/Inspector.h>
#include <boost/noncopyable.hpp>

namespace muduo
{
namespace net
{

class PerformanceInspector : boost::noncopyable
{
 public:
  void registerCommands(Inspector* ins);

  static string heap(HttpRequest::Method, const Inspector::ArgList&);
  static string growth(HttpRequest::Method, const Inspector::ArgList&);
  static string profile(HttpRequest::Method, const Inspector::ArgList&);
  static string cmdline(HttpRequest::Method, const Inspector::ArgList&);
  static string memstats(HttpRequest::Method, const Inspector::ArgList&);
  static string memhistogram(HttpRequest::Method, const Inspector::ArgList&);
  static string releaseFreeMemory(HttpRequest::Method, const Inspector::ArgList&);

  static string symbol(HttpRequest::Method, const Inspector::ArgList&);
};

}
}

#endif  // MUDUO_NET_INSPECT_PERFORMANCEINSPECTOR_H
