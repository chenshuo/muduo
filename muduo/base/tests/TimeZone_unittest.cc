#include <muduo/base/TimeZone.h>
#include <stdio.h>

using muduo::TimeZone;

int main()
{
  TimeZone tz("/etc/localtime");
}

