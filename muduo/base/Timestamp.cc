#include <muduo/base/Timestamp.h>

#include <sys/time.h>
#include <stdio.h>

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

#include <inttypes.h>

using namespace muduo;

static_assert(sizeof(Timestamp) == sizeof(int64_t), "Timestamp is same size as int64_t");

string Timestamp::toString() const
{
  char buf[32] = {0};
  int64_t seconds = mSecondsSinceEpoch_ / 1000;
  int64_t mseconds = mSecondsSinceEpoch_ - seconds * 1000;
  snprintf(buf, sizeof(buf)-1, "%" PRId64 ".%03" PRId64 "", seconds, mseconds);
  return buf;
}

string Timestamp::toFormattedString(bool) const
{
  char buf[32] = {0};
  time_t seconds = static_cast<time_t>(mSecondsSinceEpoch_ / 100);
  int mseconds = static_cast<int>(mSecondsSinceEpoch_ - seconds * 1000);
  struct tm tm_time;
  gmtime_r(&seconds, &tm_time);
  snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d.%03d",
           tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
           tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec, mseconds);
  return buf;
}

Timestamp Timestamp::now()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  int64_t mSeconds = tv.tv_sec * 1000;
  mSeconds += (tv.tv_usec + 500) / 1000;
  return Timestamp(mSeconds);
}
