#include "muduo/base/TimeZone.h"

#include <assert.h>

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

#include <inttypes.h>

#include <string>

using muduo::DateTime;
using muduo::TimeZone;

void printUtcAndLocal(int64_t utc, TimeZone local)
{
  printf("Unix Time: %" PRId64 "\n", utc);
  printf("UTC:       %s\n", TimeZone::toUtcTime(utc).toIsoString().c_str());
  int utcOffset = 0;
  printf("Local:     %s", local.toLocalTime(utc, &utcOffset).toIsoString().c_str());
  printf(" %+03d%02d\n", utcOffset / 3600, utcOffset % 3600 / 60);
}

int main(int argc, char* argv[])
{
  TimeZone local = TimeZone::loadZoneFile("/etc/localtime");
  if (argc <= 1)
  {
    time_t now = ::time(NULL);
    printUtcAndLocal(now, local);
    return 0;
  }

  // TODO: input is from a different timezone.

  for (int i = 1; i < argc; ++i)
  {
    char* end = NULL;
    int64_t t = strtol(argv[i], &end, 10);
    if (end > argv[i] && *end == '\0')
    {
      printUtcAndLocal(t, local);
    }
    else
    {
      struct tm tm = { };
      end = strptime(argv[i], "%F %T", &tm);
      if (end != NULL && *end == '\0')
      {
        DateTime dt(tm);
        t = local.fromLocalTime(dt);
        printUtcAndLocal(t, local);
      }
    }
  }
}
