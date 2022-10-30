#include "muduo/base/TimeZone.h"

#include <assert.h>

using muduo::DateTime;
using muduo::TimeZone;

int main(int argc, char* argv[])
{
  if (argc <= 1)
    return 1;
  TimeZone tz = TimeZone::loadZoneFile(argv[1]);
  assert(tz.valid());

  DateTime luck = tz.toUtcTime(1666666666);
  printf("%s\n", luck.toIsoString().c_str());
  printf("%s\n", tz.toLocalTime(1666666666).toIsoString().c_str());
}
