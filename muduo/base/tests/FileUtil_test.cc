#include <muduo/base/FileUtil.h>

#include <stdio.h>

using namespace muduo;

int main()
{
  string result;
  int64_t size = 0;
  int err = FileUtil::readFile("/proc/self", 1024, &result, &size);
  printf("%d %zd %zd\n", err, result.size(), size);
  err = FileUtil::readFile("/proc/self/cmdline", 1024, &result, &size);
  printf("%d %zd %zd\n", err, result.size(), size);
  err = FileUtil::readFile("/dev/null", 1024, &result, &size);
  printf("%d %zd %zd\n", err, result.size(), size);
  err = FileUtil::readFile("/dev/zero", 1024, &result, &size);
  printf("%d %zd %zd\n", err, result.size(), size);
  err = FileUtil::readFile("/notexist", 1024, &result, &size);
  printf("%d %zd %zd\n", err, result.size(), size);
}

