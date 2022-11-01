#include "muduo/base/TimeZone.h"
#include "muduo/base/Types.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

using muduo::DateTime;
using muduo::TimeZone;

struct tm getTm(int year, int month, int day,
                int hour, int minute, int seconds)
{
  struct tm gmt;
  muduo::memZero(&gmt, sizeof gmt);
  gmt.tm_year = year - 1900;
  gmt.tm_mon = month - 1;
  gmt.tm_mday = day;
  gmt.tm_hour = hour;
  gmt.tm_min = minute;
  gmt.tm_sec = seconds;
  return gmt;
}

struct tm getTm(const char* str)
{
  struct tm gmt;
  muduo::memZero(&gmt, sizeof gmt);
  strptime(str, "%F %T", &gmt);
  return gmt;
}

time_t getGmt(int year, int month, int day,
              int hour, int minute, int seconds)
{
  struct tm gmt = getTm(year, month, day, hour, minute, seconds);
  return timegm(&gmt);
}

time_t getGmt(const char* str)
{
  struct tm gmt = getTm(str);
  return timegm(&gmt);
}

struct TestCase
{
  const char* gmt;
  const char* local;
  bool postTransition;
};

int failure = 0;

void test(const TimeZone& tz, TestCase tc)
{
  const time_t gmt = getGmt(tc.gmt);

  {
  int utcOffset = 0;
  std::string local = tz.toLocalTime(gmt, &utcOffset).toIsoString();
  char buf[64];
  snprintf(buf, sizeof buf, " %+03d%02d", utcOffset / 3600, utcOffset % 3600 / 60);
  local += buf;

  if (local != tc.local)
  {
    printf("WRONG: ");
    printf("'%s' -> '%s' got '%s'\n", tc.gmt, tc.local, local.c_str());
    failure++;
  }
  else
  {
    printf("'%s' -> '%s'\n", tc.gmt, local.c_str());
  }
  }

  {
  struct tm local = getTm(tc.local);
  DateTime localtime(local.tm_year+1900, local.tm_mon+1, local.tm_mday,
                     local.tm_hour, local.tm_min, local.tm_sec);
  const int64_t result = tz.fromLocalTime(localtime, tc.postTransition);
  if (result != gmt)
  {
    failure++;
    printf("WRONG fromLocalTime: input %s expect %s got %s\n",
           tc.local, tc.gmt, tz.toUtcTime(result).toIsoString().c_str());
  }
  }
}

void testNewYork()
{
  TimeZone tz = TimeZone::loadZoneFile("/usr/share/zoneinfo/America/New_York");
  TestCase cases[] =
  {

    // Unix Epoch is 1969-12-31 local time.
    { "1970-01-01 00:00:00", "1969-12-31 19:00:00 -0500", false },

    { "2006-03-07 00:00:00", "2006-03-06 19:00:00 -0500", false },
    { "2006-04-02 06:59:59", "2006-04-02 01:59:59 -0500", false },
    { "2006-04-02 07:00:00", "2006-04-02 03:00:00 -0400", false },
    { "2006-05-01 00:00:00", "2006-04-30 20:00:00 -0400", false },
    { "2006-05-02 01:00:00", "2006-05-01 21:00:00 -0400", false },
    { "2006-10-21 05:00:00", "2006-10-21 01:00:00 -0400", false },
    { "2006-10-29 05:59:59", "2006-10-29 01:59:59 -0400", false },
    { "2006-10-29 06:00:00", "2006-10-29 01:00:00 -0500", true },
    { "2006-10-29 06:30:00", "2006-10-29 01:30:00 -0500", true },
    { "2006-12-31 06:00:00", "2006-12-31 01:00:00 -0500", false },
    { "2007-01-01 00:00:00", "2006-12-31 19:00:00 -0500", false },

    { "2007-03-07 00:00:00", "2007-03-06 19:00:00 -0500", false },
    { "2007-03-11 06:59:59", "2007-03-11 01:59:59 -0500", false },
    { "2007-03-11 07:00:00", "2007-03-11 03:00:00 -0400", false },
    { "2007-05-01 00:00:00", "2007-04-30 20:00:00 -0400", false },
    { "2007-05-02 01:00:00", "2007-05-01 21:00:00 -0400", false },
    { "2007-10-31 05:00:00", "2007-10-31 01:00:00 -0400", false },
    { "2007-11-04 05:59:59", "2007-11-04 01:59:59 -0400", false },
    { "2007-11-04 06:00:00", "2007-11-04 01:00:00 -0500", true },
    { "2007-11-04 06:59:59", "2007-11-04 01:59:59 -0500", true },
    { "2007-12-31 06:00:00", "2007-12-31 01:00:00 -0500", false },
    { "2008-01-01 00:00:00", "2007-12-31 19:00:00 -0500", false },

    { "2009-03-07 00:00:00", "2009-03-06 19:00:00 -0500", false },
    { "2009-03-08 06:59:59", "2009-03-08 01:59:59 -0500", false },
    { "2009-03-08 07:00:00", "2009-03-08 03:00:00 -0400", false },
    { "2009-05-01 00:00:00", "2009-04-30 20:00:00 -0400", false },
    { "2009-05-02 01:00:00", "2009-05-01 21:00:00 -0400", false },
    { "2009-10-31 05:00:00", "2009-10-31 01:00:00 -0400", false },
    { "2009-11-01 05:59:59", "2009-11-01 01:59:59 -0400", false },
    { "2009-11-01 06:00:00", "2009-11-01 01:00:00 -0500", true },
    { "2009-11-01 06:59:59", "2009-11-01 01:59:59 -0500", true },
    { "2009-12-31 06:00:00", "2009-12-31 01:00:00 -0500", false },
    { "2010-01-01 00:00:00", "2009-12-31 19:00:00 -0500", false },

    { "2010-03-13 00:00:00", "2010-03-12 19:00:00 -0500", false },
    { "2010-03-14 06:59:59", "2010-03-14 01:59:59 -0500", false },
    { "2010-03-14 07:00:00", "2010-03-14 03:00:00 -0400", false },
    { "2010-05-01 00:00:00", "2010-04-30 20:00:00 -0400", false },
    { "2010-05-02 01:00:00", "2010-05-01 21:00:00 -0400", false },
    { "2010-11-06 05:00:00", "2010-11-06 01:00:00 -0400", false },
    { "2010-11-07 05:59:59", "2010-11-07 01:59:59 -0400", false },
    { "2010-11-07 06:00:00", "2010-11-07 01:00:00 -0500", true },
    { "2010-11-07 06:59:59", "2010-11-07 01:59:59 -0500", true },
    { "2010-12-31 06:00:00", "2010-12-31 01:00:00 -0500", false },
    { "2011-01-01 00:00:00", "2010-12-31 19:00:00 -0500", false },

    { "2011-03-01 00:00:00", "2011-02-28 19:00:00 -0500", false },
    { "2011-03-13 06:59:59", "2011-03-13 01:59:59 -0500", false },
    { "2011-03-13 07:00:00", "2011-03-13 03:00:00 -0400", false },
    { "2011-05-01 00:00:00", "2011-04-30 20:00:00 -0400", false },
    { "2011-05-02 01:00:00", "2011-05-01 21:00:00 -0400", false },
    { "2011-11-06 05:59:59", "2011-11-06 01:59:59 -0400", false },
    { "2011-11-06 06:00:00", "2011-11-06 01:00:00 -0500", true },
    { "2011-11-06 06:59:59", "2011-11-06 01:59:59 -0500", true },
    { "2011-12-31 06:00:00", "2011-12-31 01:00:00 -0500", false },
    { "2012-01-01 00:00:00", "2011-12-31 19:00:00 -0500", false },

  };

  for (const auto& c : cases)
  {
    test(tz, c);
  }
}

void testLondon()
{
  // UTC time             isdst  offset  Local time (London)
  // 2010-03-28 01:00:00Z   1     1.0    2010-03-28 02:00:00
  // 2010-10-31 01:00:00Z   0     0.0    2010-10-31 01:00:00
  // 2011-03-27 01:00:00Z   1     1.0    2011-03-27 02:00:00
  // 2011-10-30 01:00:00Z   0     0.0    2011-10-30 01:00:00
  // 2012-03-25 01:00:00Z   1     1.0    2012-03-25 02:00:00
  // 2012-10-28 01:00:00Z   0     0.0    2012-10-28 01:00:00
  // 2013-03-31 01:00:00Z   1     1.0    2013-03-31 02:00:00
  // 2013-10-27 01:00:00Z   0     0.0    2013-10-27 01:00:00
  // 2014-03-30 01:00:00Z   1     1.0    2014-03-30 02:00:00
  // 2014-10-26 01:00:00Z   0     0.0    2014-10-26 01:00:00

  TimeZone tz = TimeZone::loadZoneFile("/usr/share/zoneinfo/Europe/London");
  TestCase cases[] =
  {

    { "2011-03-26 00:00:00", "2011-03-26 00:00:00 +0000", false },
    { "2011-03-27 00:59:59", "2011-03-27 00:59:59 +0000", false },
    { "2011-03-27 01:00:00", "2011-03-27 02:00:00 +0100", false },
    { "2011-10-30 00:59:59", "2011-10-30 01:59:59 +0100", false },
    { "2011-10-30 01:00:00", "2011-10-30 01:00:00 +0000", true },
    { "2011-10-30 01:59:59", "2011-10-30 01:59:59 +0000", true },
    { "2011-12-31 22:00:00", "2011-12-31 22:00:00 +0000", false },
    { "2012-01-01 00:00:00", "2012-01-01 00:00:00 +0000", false },

    { "2012-03-24 00:00:00", "2012-03-24 00:00:00 +0000", false },
    { "2012-03-25 00:59:59", "2012-03-25 00:59:59 +0000", false },
    { "2012-03-25 01:00:00", "2012-03-25 02:00:00 +0100", false },
    { "2012-10-28 00:59:59", "2012-10-28 01:59:59 +0100", false },
    { "2012-10-28 01:00:00", "2012-10-28 01:00:00 +0000", true },
    { "2012-10-28 01:59:59", "2012-10-28 01:59:59 +0000", true },
    { "2012-12-31 22:00:00", "2012-12-31 22:00:00 +0000", false },
    { "2013-01-01 00:00:00", "2013-01-01 00:00:00 +0000", false },

  };

  for (const auto& c : cases)
  {
    test(tz, c);
  }
}

void testHongKong()
{
  TimeZone tz = TimeZone::loadZoneFile("/usr/share/zoneinfo/Asia/Hong_Kong");
  TestCase cases[] =
  {

    { "2011-04-03 00:00:00", "2011-04-03 08:00:00 +0800", false},

  };

  for (const auto& c : cases)
  {
    test(tz, c);
  }

  tz = TimeZone::loadZoneFile("/usr/share/zoneinfo/PRC");
  for (const auto& c : cases)
  {
    test(tz, c);
  }

}

void testSydney()
{
  // DST starts in winter
  // UTC time             isdst  offset  Local time (London)
  // 2010-04-03 16:00:00Z isdst 0 offset  10.0  2010-04-04 02:00:00
  // 2010-10-02 16:00:00Z isdst 1 offset  11.0  2010-10-03 03:00:00
  // 2011-04-02 16:00:00Z isdst 0 offset  10.0  2011-04-03 02:00:00
  // 2011-10-01 16:00:00Z isdst 1 offset  11.0  2011-10-02 03:00:00
  // 2012-03-31 16:00:00Z isdst 0 offset  10.0  2012-04-01 02:00:00
  // 2012-10-06 16:00:00Z isdst 1 offset  11.0  2012-10-07 03:00:00

  TimeZone tz = TimeZone::loadZoneFile("/usr/share/zoneinfo/Australia/Sydney");
  TestCase cases[] =
  {

    { "2011-01-01 00:00:00", "2011-01-01 11:00:00 +1100", false },
    { "2011-04-02 15:59:59", "2011-04-03 02:59:59 +1100", false },
    { "2011-04-02 16:00:00", "2011-04-03 02:00:00 +1000", true },
    { "2011-04-02 16:59:59", "2011-04-03 02:59:59 +1000", true },
    { "2011-05-02 01:00:00", "2011-05-02 11:00:00 +1000", false },
    { "2011-10-01 15:59:59", "2011-10-02 01:59:59 +1000", false },
    { "2011-10-01 16:00:00", "2011-10-02 03:00:00 +1100", false },
    { "2011-12-31 22:00:00", "2012-01-01 09:00:00 +1100", false },

  };

  for (const auto& c : cases)
  {
    test(tz, c);
  }
}

void testUtc()
{
  TimeZone utc = TimeZone::loadZoneFile("/usr/share/zoneinfo/UTC");
  const int kRange = 100*1000*1000;
  for (time_t t = -kRange; t <= kRange; t += 11)
  {
    struct tm* t1 = gmtime(&t);
    char buf[80];
    strftime(buf, sizeof buf, "%F %T", t1);

    struct DateTime t2 = TimeZone::toUtcTime(t);
    std::string t2str = t2.toIsoString();
    if (t2str != buf)
    {
      printf("'%s' != '%s'\n", buf, t2str.c_str());
      failure++;
      assert(0);
    }

    struct DateTime t3 = utc.toLocalTime(t);
    std::string t3str = t3.toIsoString();
    if (t3str != buf)
    {
      printf("'%s' != '%s'\n", buf, t3str.c_str());
      failure++;
      assert(0);
    }


    int64_t u1 = TimeZone::fromUtcTime(t2);
    if (t != u1)
    {
      printf("%lld != %lld\n", static_cast<long long>(t), static_cast<long long>(u1));
      failure++;
      assert(0);
    }
  }
}

void testFixedTimezone()
{
  TimeZone tz(8*3600, "CST");
  TestCase cases[] =
  {
    { "2014-04-03 00:00:00", "2014-04-03 08:00:00 +0800", false},
  };

  for (const auto& c : cases)
  {
    test(tz, c);
  }

  tz = TimeZone::loadZoneFile("/usr/share/zoneinfo/Etc/GMT-8");
  for (const auto& c : cases)
  {
    test(tz, c);
  }

}

struct LocalToUtcTestCase
{
  const char* gmt;
  const char* local;
  bool postTransition;
};

void testLosAngeles()
{
  // UTC time             isdst  offset  Local time           Prior second local time
  // 2021-03-14 10:00:00Z   1     -7.0   2021-03-14 03:00:00
  // 2021-11-07 09:00:00Z   0     -8.0   2021-11-07 01:00:00
  // 2022-03-13 10:00:00Z   1     -7.0   2022-03-13 03:00:00  2022-03-13 01:59:59
  // 2022-11-06 09:00:00Z   0     -8.0   2022-11-06 01:00:00  2022-11-06 01:59:59
  // 2023-03-12 10:00:00Z   1     -7.0   2023-03-12 03:00:00
  // 2023-11-05 09:00:00Z   0     -8.0   2023-11-05 01:00:00

  TimeZone tz = TimeZone::loadZoneFile("/usr/share/zoneinfo/America/Los_Angeles");
  int utcOffset = 0;
  printf("1234567890 in Los Angeles: %s", tz.toLocalTime(1234567890, &utcOffset).toIsoString().c_str());
  printf(" %+03d%02d\n", utcOffset / 3600, utcOffset % 3600 / 60);
  printf("1666666666 in Los Angeles: %s\n", tz.toLocalTime(1666666666).toIsoString().c_str());
  printf("Now in Los Angeles: %s\n", tz.toLocalTime(time(nullptr)).toIsoString().c_str());

  LocalToUtcTestCase cases[] =
  {
    // Unix Epoch is 1969-12-31 local time.
    {"1970-01-01 00:00:00", "1969-12-31 16:00:00 -0800", false },
    {"1970-01-01 00:00:00", "1969-12-31 16:00:00 -0800", true },

    {"2022-01-01 18:00:00", "2022-01-01 10:00:00", false},
    {"2022-01-01 18:00:00", "2022-01-01 10:00:00", true },
    // Before DST
    {"2022-03-13 09:00:00", "2022-03-13 01:00:00", false },
    {"2022-03-13 09:00:00", "2022-03-13 01:00:00", true },
    {"2022-03-13 09:59:59", "2022-03-13 01:59:59", false },
    {"2022-03-13 09:59:59", "2022-03-13 01:59:59", true },
    // local time doesn't exist, skipped
    {"2022-03-13 10:00:00", "2022-03-13 02:00:00", false },
    {"2022-03-13 09:00:00", "2022-03-13 02:00:00", true },
    {"2022-03-13 10:59:59", "2022-03-13 02:59:59", false },
    {"2022-03-13 09:59:59", "2022-03-13 02:59:59", true },
    // in DST
    {"2022-03-13 10:00:00", "2022-03-13 03:00:00", false },
    {"2022-03-13 10:00:00", "2022-03-13 03:00:00", true },
    // Before back to winter time
    {"2022-11-06 07:59:59", "2022-11-06 00:59:59", false },
    {"2022-11-06 07:59:59", "2022-11-06 00:59:59", true },
    // Time repeats
    {"2022-11-06 08:00:00", "2022-11-06 01:00:00", false },
    {"2022-11-06 09:00:00", "2022-11-06 01:00:00", true },
    {"2022-11-06 08:59:59", "2022-11-06 01:59:59", false },
    {"2022-11-06 09:59:59", "2022-11-06 01:59:59", true },
    // After DST
    {"2022-11-06 10:00:00", "2022-11-06 02:00:00", false },
    {"2022-11-06 10:00:00", "2022-11-06 02:00:00", true },
    {"2023-01-01 06:00:00", "2022-12-31 22:00:00", false },
    {"2023-01-01 06:00:00", "2022-12-31 22:00:00", true },
  };

  for (const auto& tc : cases)
  {
    int64_t gmt = getGmt(tc.gmt);
    struct tm local = getTm(tc.local);
    DateTime localtime(local.tm_year+1900, local.tm_mon+1, local.tm_mday,
                       local.tm_hour, local.tm_min, local.tm_sec);
    int64_t actual = tz.fromLocalTime(localtime, tc.postTransition);
    printf("Local %s -> %sZ\n", tc.local, tc.gmt);
    if (gmt != actual)
    {
      failure++;
      printf("WRONG: input %s post %d expect %s got %s\n",
             tc.local, tc.postTransition, tc.gmt,
             tz.toUtcTime(actual).toIsoString().c_str());
    }
  }

  int64_t start = getGmt("1950-01-01 00:00:00");
  int64_t end = getGmt("2037-01-01 00:00:00");
  for (int64_t utc = start; utc <= end; utc += 30)
  {
    DateTime local = tz.toLocalTime(utc);
    int64_t gmt = tz.fromLocalTime(local);
    if (utc != gmt)
    {
      // try post transistion
      int64_t post = tz.fromLocalTime(local, true);
      if (post != utc)
      {
        failure++;
        printf("WRONG: input %s local %s got %s\n",
               tz.toUtcTime(utc).toIsoString().c_str(),
               local.toIsoString().c_str(),
               tz.toUtcTime(gmt).toIsoString().c_str());
      }
    }
  }
}

int main()
{
  testLosAngeles();
  testNewYork();
  testLondon();
  testSydney();
  testHongKong();
  testFixedTimezone();
  testUtc();

  return failure;
}
