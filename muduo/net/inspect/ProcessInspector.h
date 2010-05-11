// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is an internal header file, you should not include this.

#ifndef MUDUO_NET_INSPECT_PROCESSINSPECTOR_H
#define MUDUO_NET_INSPECT_PROCESSINSPECTOR_H

#include <muduo/net/inspect/Inspector.h>
#include <boost/noncopyable.hpp>

namespace muduo
{
namespace net
{

class ProcessInspector : boost::noncopyable
{
 public:
  void registerCommands(Inspector* ins);

 private:
  static string procStatus(const Inspector::ArgList& args);
  static string openedFiles(const Inspector::ArgList& args);

};

}
}

#endif  // MUDUO_NET_INSPECT_PROCESSINSPECTOR_H
