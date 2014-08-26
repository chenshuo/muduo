// Copyright 2014, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//

#include <muduo/net/inspect/SystemInspector.h>
#include <muduo/base/FileUtil.h>

#include <sys/utsname.h>

using namespace muduo;
using namespace muduo::net;

namespace muduo
{
namespace inspect
{

string uptime(Timestamp now, Timestamp start, bool showMicroseconds);
long getLong(const string& content, const char* key);
int stringPrintf(string* out, const char* fmt, ...) __attribute__ ((format (printf, 2, 3)));

}
}

using namespace muduo::inspect;

void SystemInspector::registerCommands(Inspector* ins)
{
  ins->add("sys", "overview", SystemInspector::overview, "print system overview");
  ins->add("sys", "loadavg", SystemInspector::loadavg, "print /proc/loadavg");
  ins->add("sys", "version", SystemInspector::version, "print /proc/version");
  ins->add("sys", "cpuinfo", SystemInspector::cpuinfo, "print /proc/cpuinfo");
  ins->add("sys", "meminfo", SystemInspector::meminfo, "print /proc/meminfo");
  ins->add("sys", "stat", SystemInspector::stat, "print /proc/stat");
}

string SystemInspector::loadavg(HttpRequest::Method, const Inspector::ArgList&)
{
  string loadavg;
  FileUtil::readFile("/proc/loadavg", 65536, &loadavg);
  return loadavg;
}

string SystemInspector::version(HttpRequest::Method, const Inspector::ArgList&)
{
  string version;
  FileUtil::readFile("/proc/version", 65536, &version);
  return version;
}

string SystemInspector::cpuinfo(HttpRequest::Method, const Inspector::ArgList&)
{
  string cpuinfo;
  FileUtil::readFile("/proc/cpuinfo", 65536, &cpuinfo);
  return cpuinfo;
}

string SystemInspector::meminfo(HttpRequest::Method, const Inspector::ArgList&)
{
  string meminfo;
  FileUtil::readFile("/proc/meminfo", 65536, &meminfo);
  return meminfo;
}

string SystemInspector::stat(HttpRequest::Method, const Inspector::ArgList&)
{
  string stat;
  FileUtil::readFile("/proc/stat", 65536, &stat);
  return stat;
}

string SystemInspector::overview(HttpRequest::Method, const Inspector::ArgList&)
{
  string result;
  result.reserve(1024);
  Timestamp now = Timestamp::now();
  result += "Page generated at ";
  result += now.toFormattedString();
  result += " (UTC)\n";
  // Hardware and OS
  {
  struct utsname un;
  if (::uname(&un) == 0)
  {
    stringPrintf(&result, "Hostname: %s\n", un.nodename);
    stringPrintf(&result, "Machine: %s\n", un.machine);
    stringPrintf(&result, "OS: %s %s %s\n", un.sysname, un.release, un.version);
  }
  }
  string stat;
  FileUtil::readFile("/proc/stat", 65536, &stat);
  Timestamp bootTime(Timestamp::kMicroSecondsPerSecond * getLong(stat, "btime "));
  result += "Boot time: ";
  result += bootTime.toFormattedString(false /* show microseconds */);
  result += " (UTC)\n";
  result += "Up time: ";
  result += uptime(now, bootTime, false /* show microseconds */);
  result += "\n";

  // CPU load
  {
  string loadavg;
  FileUtil::readFile("/proc/loadavg", 65536, &loadavg);
  stringPrintf(&result, "Processes created: %ld\n", getLong(stat, "processes "));
  stringPrintf(&result, "Loadavg: %s\n", loadavg.c_str());
  }

  // Memory
  {
  string meminfo;
  FileUtil::readFile("/proc/meminfo", 65536, &meminfo);
  long total_kb = getLong(meminfo, "MemTotal:");
  long free_kb = getLong(meminfo, "MemFree:");
  long buffers_kb = getLong(meminfo, "Buffers:");
  long cached_kb = getLong(meminfo, "Cached:");

  stringPrintf(&result, "Total Memory: %6ld MiB\n", total_kb / 1024);
  stringPrintf(&result, "Free Memory:  %6ld MiB\n", free_kb / 1024);
  stringPrintf(&result, "Buffers:      %6ld MiB\n", buffers_kb / 1024);
  stringPrintf(&result, "Cached:       %6ld MiB\n", cached_kb / 1024);
  stringPrintf(&result, "Real Used:    %6ld MiB\n", (total_kb - free_kb - buffers_kb - cached_kb) / 1024);
  stringPrintf(&result, "Real Free:    %6ld MiB\n", (free_kb + buffers_kb + cached_kb) / 1024);

  // Swap
  }
  // Disk
  // Network
  return result;
}
