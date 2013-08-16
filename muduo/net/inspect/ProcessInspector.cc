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
#include <stdarg.h>
#include <unistd.h>

using namespace muduo;
using namespace muduo::net;

string uptime()
{
  char buf[256];
  int seconds = static_cast<int>(timeDifference(Timestamp::now(), ProcessInfo::startTime()));
  int days = seconds/86400;
  int hours = (seconds % 86400) / 3600;
  int minutes = (seconds % 3600) / 60;
  snprintf(buf, sizeof buf, "%d days %02d:%02d:%02d", days, hours, minutes, seconds % 60);
  return buf;
}

long getLong(const string& procStatus, const char* key)
{
  long result = 0;
  size_t pos = procStatus.find(key);
  if (pos != string::npos)
  {
    result = ::atol(procStatus.c_str() + pos + strlen(key));
  }
  return result;
}

string getProcessName(const string& procStatus)
{
  string result;
  size_t pos = procStatus.find("Name:");
  if (pos != string::npos)
  {
    pos += strlen("Name:");
    while (procStatus[pos] == '\t')
      ++pos;
    size_t eol = pos;
    while (procStatus[eol] != '\n')
      ++eol;
    result = procStatus.substr(pos, eol-pos);
  }
  return result;
}

long getStatField(const string& procStat, int field)
{
  // 969 (inspector_test) S
  // 19089 969 19089 34823 969 4202496 609 0 0 0 0 3 0 0
  // 20 0 2 0 162443763 95813632 463 18446744073709551615 4194304 5159121
  // 140737297994192 140737297972304 140050830496611 0 0 4096 0
  // 18446744073709551615 0 0 17 3 0 0 0 0 0
  //
  long result = -1;
  size_t pos = procStat.find(") ");
  if (pos != string::npos)
  {
    const char* start = procStat.c_str() + pos + 4;
    int f = 0;
    const char* end = &*procStat.end();
    // FIXME: Test with last field
    while (f < field && start < end)
    {
      while (start < end && *start != ' ')
        ++start;
      ++f;
      while (start < end && *start == ' ')
        ++start;
    }
    if (f == field)
    {
      result = strtol(start, NULL, 10);
    }
  }
  return result;
}

int stringPrintf(string* out, const char* fmt, ...) __attribute__ ((format (printf, 2, 3)));

int stringPrintf(string* out, const char* fmt, ...)
{
  char buf[256];
  va_list args;
  va_start(args, fmt);
  int ret = vsnprintf(buf, sizeof buf, fmt, args);
  va_end(args);
  out->append(buf);
  return ret;
}

string ProcessInspector::username_ = ProcessInfo::username();

void ProcessInspector::registerCommands(Inspector* ins)
{
  ins->add("proc", "basic", ProcessInspector::overview, "print basic overview");
  ins->add("proc", "pid", ProcessInspector::pid, "print pid");
  ins->add("proc", "status", ProcessInspector::procStatus, "print /proc/self/status");
  ins->add("proc", "opened_files", ProcessInspector::openedFiles, "count /proc/self/fd");
  ins->add("proc", "threads", ProcessInspector::threads, "list /proc/self/task");
}

string ProcessInspector::overview(HttpRequest::Method, const Inspector::ArgList&)
{
  string result;
  result.reserve(1024);
  result += "Page generated at ";
  result += Timestamp::now().toFormattedString();
  result += " (UTC)\nStarted at ";
  result += ProcessInfo::startTime().toFormattedString();
  result += " (UTC), up for ";
  result += uptime();
  result += "\n";

  string procStatus = ProcessInfo::procStatus();
  result += getProcessName(procStatus);
  result += " (";
  result += ProcessInfo::exePath();
  result += ") running as ";
  result += username_;
  result += " on ";
  result += ProcessInfo::hostname(); // cache ?
  result += "\n";

  stringPrintf(&result, "pid %d, num of threads %ld\n",
               ProcessInfo::pid(), getLong(procStatus, "Threads:"));

  result += "Virtual memory: ";
  stringPrintf(&result, "%.3f MiB, ",
               static_cast<double>(getLong(procStatus, "VmSize:")) / 1024.0);

  result += "RSS memory: ";
  stringPrintf(&result, "%.3f MiB\n",
               static_cast<double>(getLong(procStatus, "VmRSS:")) / 1024.0);

  stringPrintf(&result, "Opened files: %d, limit: %d\n",
               ProcessInfo::openedFiles(), ProcessInfo::maxOpenFiles());

  string procStat = ProcessInfo::procStat();

  /*
  stringPrintf(&result, "ppid %ld\n", getStatField(procStat, 0));
  stringPrintf(&result, "pgid %ld\n", getStatField(procStat, 1));
  */

  double clk = static_cast<double>(sysconf(_SC_CLK_TCK));
  stringPrintf(&result, "User time: %.3fs, Sys time: %.3fs\n",
               static_cast<double>(getStatField(procStat, 10)) / clk,
               static_cast<double>(getStatField(procStat, 11)) / clk);

  // FIXME: add context switches

  return result;
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

