// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//

#include <muduo/base/ProcessInfo.h>

#include <algorithm>

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

using namespace muduo;

namespace
{
  __thread int t_numOpenedFiles = 0;
  int fdDirFilter(const struct dirent*)
  {
    ++t_numOpenedFiles;
    return 0;
  }

  __thread std::vector<pid_t>* t_pids = 0;
  int taskDirFilter(const struct dirent* d)
  {
    if (isdigit(d->d_name[0]))
    {
      t_pids->push_back(atoi(d->d_name));
    }
    return 0;
  }
}

pid_t ProcessInfo::pid()
{
  return ::getpid();
}

string ProcessInfo::procStatus()
{
  string result;
  FILE* fp = fopen("/proc/self/status", "r");
  if (fp)
  {
    while (!feof(fp))
    {
      char buf[8192];
      size_t n = fread(buf, 1, sizeof buf, fp);
      result.append(buf, n);
    }
    fclose(fp);
  }
  return result;
}

int ProcessInfo::openedFiles()
{
  t_numOpenedFiles = 0;
  struct dirent** namelist;
  scandir("/proc/self/fd", &namelist, fdDirFilter, alphasort);
  return t_numOpenedFiles-2; // "." and ".."
}

std::vector<pid_t> ProcessInfo::threads()
{
  std::vector<pid_t> result;
  t_pids = &result;
  struct dirent** namelist;
  scandir("/proc/self/task", &namelist, taskDirFilter, alphasort);
  std::sort(result.begin(), result.end());
  return result;
}
