#include <muduo/base/Timestamp.h>

#include <sys/time.h>
#include <stdio.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#undef __STDC_FORMAT_MACROS

#include <boost/static_assert.hpp>

using namespace muduo;

BOOST_STATIC_ASSERT(sizeof(Timestamp) == sizeof(int64_t));

Timestamp::Timestamp()
  : microSecondsSinceEpoch_(0)
{
}

Timestamp::Timestamp(int64_t microseconds)
  : microSecondsSinceEpoch_(microseconds)
{
}

string Timestamp::toString() const
{
  char buf[32] = {0};
  int64_t seconds = microSecondsSinceEpoch_ / kMicroSecondsPerSecond;
  int64_t microseconds = microSecondsSinceEpoch_ % kMicroSecondsPerSecond;
  snprintf(buf, sizeof(buf)-1, "%" PRId64 ".%06" PRId64 "", seconds, microseconds);
  return buf;
}

Timestamp Timestamp::now()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  int64_t seconds = tv.tv_sec;
  return Timestamp(seconds * kMicroSecondsPerSecond + tv.tv_usec);
}

Timestamp Timestamp::invalid()
{
  return Timestamp();
}

