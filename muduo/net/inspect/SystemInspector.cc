// Copyright 2014, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//

#include <muduo/net/inspect/SystemInspector.h>
#include <muduo/base/FileUtil.h>

using namespace muduo;
using namespace muduo::net;

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
  return "Not implemented.";
}
