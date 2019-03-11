// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//

#include "muduo/net/inspect/ProcessInspector.h"
#include "muduo/base/FileUtil.h"
#include "muduo/base/ProcessInfo.h"
#include <limits.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>

using namespace muduo;
using namespace muduo::net;

namespace muduo
{
namespace inspect
{

string uptime(Timestamp now, Timestamp start, bool showMicroseconds)
{
  char buf[256];
  int64_t age = now.microSecondsSinceEpoch() - start.microSecondsSinceEpoch();
  int seconds = static_cast<int>(age / Timestamp::kMicroSecondsPerSecond);
  int days = seconds/86400;
  int hours = (seconds % 86400) / 3600;
  int minutes = (seconds % 3600) / 60;
  if (showMicroseconds)
  {
    int microseconds = static_cast<int>(age % Timestamp::kMicroSecondsPerSecond);
    snprintf(buf, sizeof buf, "%d days %02d:%02d:%02d.%06d",
             days, hours, minutes, seconds % 60, microseconds);
  }
  else
  {
    snprintf(buf, sizeof buf, "%d days %02d:%02d:%02d",
             days, hours, minutes, seconds % 60);
  }
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

StringPiece next(StringPiece data)
{
  const char* sp = static_cast<const char*>(::memchr(data.data(), ' ', data.size()));
  if (sp)
  {
    data.remove_prefix(static_cast<int>(sp+1-data.begin()));
    return data;
  }
  return "";
}

ProcessInfo::CpuTime getCpuTime(StringPiece data)
{
  ProcessInfo::CpuTime t;

  for (int i = 0; i < 10; ++i)
  {
    data = next(data);
  }
  long utime = strtol(data.data(), NULL, 10);
  data = next(data);
  long stime = strtol(data.data(), NULL, 10);
  const double hz = static_cast<double>(ProcessInfo::clockTicksPerSecond());
  t.userSeconds = static_cast<double>(utime) / hz;
  t.systemSeconds = static_cast<double>(stime) / hz;
  return t;
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

}  // namespace inspect
}  // namespace muduo

using namespace muduo::inspect;

string ProcessInspector::username_ = ProcessInfo::username();

void ProcessInspector::registerCommands(Inspector* ins)
{
  ins->add("proc", "overview", ProcessInspector::overview, "print basic overview");
  ins->add("proc", "pid", ProcessInspector::pid, "print pid");
  ins->add("proc", "status", ProcessInspector::procStatus, "print /proc/self/status");
  // ins->add("proc", "opened_files", ProcessInspector::openedFiles, "count /proc/self/fd");
  ins->add("proc", "threads", ProcessInspector::threads, "list /proc/self/task");
}

string ProcessInspector::overview(HttpRequest::Method, const Inspector::ArgList&)
{
  string result;
  result.reserve(1024);
  Timestamp now = Timestamp::now();
  result += "Page generated at ";
  result += now.toFormattedString();
  result += " (UTC)\nStarted at ";
  result += ProcessInfo::startTime().toFormattedString();
  result += " (UTC), up for ";
  result += uptime(now, ProcessInfo::startTime(), true/* show microseconds */);
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

  if (ProcessInfo::isDebugBuild())
  {
    result += "WARNING: debug build!\n";
  }

  stringPrintf(&result, "pid %d, num of threads %ld, bits %zd\n",
               ProcessInfo::pid(), getLong(procStatus, "Threads:"), CHAR_BIT * sizeof(void*));

  result += "Virtual memory: ";
  stringPrintf(&result, "%.3f MiB, ",
               static_cast<double>(getLong(procStatus, "VmSize:")) / 1024.0);

  result += "RSS memory: ";
  stringPrintf(&result, "%.3f MiB\n",
               static_cast<double>(getLong(procStatus, "VmRSS:")) / 1024.0);

  // FIXME: VmData:

  stringPrintf(&result, "Opened files: %d, limit: %d\n",
               ProcessInfo::openedFiles(), ProcessInfo::maxOpenFiles());

  // string procStat = ProcessInfo::procStat();

  /*
  stringPrintf(&result, "ppid %ld\n", getStatField(procStat, 0));
  stringPrintf(&result, "pgid %ld\n", getStatField(procStat, 1));
  */

  ProcessInfo::CpuTime t = ProcessInfo::cpuTime();
  stringPrintf(&result, "User time: %12.3fs\nSys time:  %12.3fs\n",
               t.userSeconds, t.systemSeconds);

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
  string result = "  TID NAME             S    User Time  System Time\n";
  result.reserve(threads.size() * 64);
  string stat;
  for (pid_t tid : threads)
  {
    char buf[256];
    snprintf(buf, sizeof buf, "/proc/%d/task/%d/stat", ProcessInfo::pid(), tid);
    if (FileUtil::readFile(buf, 65536, &stat) == 0)
    {
      StringPiece name = ProcessInfo::procname(stat);
      const char* rp = name.end();
      assert(*rp == ')');
      const char* state = rp + 2;
      *const_cast<char*>(rp) = '\0';  // don't do this at home
      StringPiece data(stat);
      data.remove_prefix(static_cast<int>(state - data.data() + 2));
      ProcessInfo::CpuTime t = getCpuTime(data);
      snprintf(buf, sizeof buf, "%5d %-16s %c %12.3f %12.3f\n",
               tid, name.data(), *state, t.userSeconds, t.systemSeconds);
      result += buf;
    }
  }
  return result;
}

