// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//

#include <muduo/net/inspect/ProcessInfo.h>
#include <stdio.h>

using namespace muduo;
using namespace muduo::net;

void ProcessInfo::registerCommands(Inspector* ins)
{
  ins->add("proc", "status", ProcessInfo::procStatus, "print /proc/self/status");
}

string ProcessInfo::procStatus(const Inspector::ArgList& args)
{
  string result;
  FILE* fp = fopen("/proc/self/status", "r");
  if (fp)
  {
    char buf[256] = {0};
    while (fgets(buf, sizeof(buf), fp) != NULL)
    {
      result += buf;
    }
    fclose(fp);
  }
  return result;
}

