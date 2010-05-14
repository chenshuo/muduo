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
  ins->add("proc", "pid", ProcessInspector::pid, "print pid");
  ins->add("proc", "status", ProcessInspector::procStatus, "print /proc/self/status");
  ins->add("proc", "opened_files", ProcessInspector::openedFiles, "count /proc/self/fd");
  ins->add("proc", "threads", ProcessInspector::threads, "list /proc/self/task");
}

string ProcessInspector::pid(HttpRequest::Method, const Inspector::ArgList&)
{
  char buf[32];
  snprintf(buf, sizeof buf, "%d", ProcessInfo::pid());
  return buf;
}

string ProcessInspector::procStatus(HttpRequest::Method, const Inspector::ArgList&)
{
  return ProcessInfo::procStatus();
}

string ProcessInspector::openedFiles(HttpRequest::Method, const Inspector::ArgList&)
{
  char buf[32];
  snprintf(buf, sizeof buf, "%d", ProcessInfo::openedFiles());
  return buf;
}

string ProcessInspector::threads(HttpRequest::Method, const Inspector::ArgList&)
{
  std::vector<pid_t> threads = ProcessInfo::threads();
  string result;
  for (size_t i = 0; i < threads.size(); ++i)
  {
    char buf[32];
    snprintf(buf, sizeof buf, "%d\n", threads[i]);
    result += buf;
  }
  return result;
}

