// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//

#include <muduo/net/inspect/ProcessInspector.h>
#include <muduo/base/ProcessInfo.h>
#include <stdio.h>

using namespace muduo;
using namespace muduo::net;

void ProcessInspector::registerCommands(Inspector* ins)
{
  ins->add("proc", "status", ProcessInspector::procStatus, "print /proc/self/status");
  ins->add("proc", "opened_files", ProcessInspector::openedFiles, "count /proc/self/fd");
}

string ProcessInspector::procStatus(const Inspector::ArgList& args)
{
  return ProcessInfo::procStatus();
}

string ProcessInspector::openedFiles(const Inspector::ArgList& args)
{
  char buf[32];
  snprintf(buf, sizeof buf, "%d", ProcessInfo::openedFiles());
  return buf;
}
