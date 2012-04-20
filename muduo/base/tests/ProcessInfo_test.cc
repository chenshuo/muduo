#include <muduo/base/ProcessInfo.h>
#include <stdio.h>

int main()
{
  printf("pid = %d\n", muduo::ProcessInfo::pid());
  printf("uid = %d\n", muduo::ProcessInfo::uid());
  printf("euid = %d\n", muduo::ProcessInfo::euid());
  printf("hostname = %s\n", muduo::ProcessInfo::hostname().c_str());
  printf("opened files = %d\n", muduo::ProcessInfo::openedFiles());
  printf("threads = %zd\n", muduo::ProcessInfo::threads().size());
}
