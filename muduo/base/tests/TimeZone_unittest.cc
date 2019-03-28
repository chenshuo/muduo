#include "muduo/base/TimeZone.h"
#include "muduo/base/Types.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

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
  bool isdst;
};

void test(const TimeZone& tz, TestCase tc)
{
  time_t gmt = getGmt(tc.gmt);

  {
  struct tm local = tz.toLocalTime(gmt);
  char buf[256];
  strftime(buf, sizeof buf, "%F %T%z(%Z)", &local);

  if (strcmp(buf, tc.local) != 0 || tc.isdst != local.tm_isdst)
  {
    printf("WRONG: ");
  }
  printf("%s -> %s\n", tc.gmt, buf);
  }

  {
  struct tm local = getTm(tc.local);
  local.tm_isdst = tc.isdst;
  time_t result = tz.fromLocalTime(local);
  if (result != gmt)
  {
    struct tm local2 = tz.toLocalTime(result);
    char buf[256];
    strftime(buf, sizeof buf, "%F %T%z(%Z)", &local2);

    printf("WRONG fromLocalTime: %ld %ld %s\n",
           static_cast<long>(gmt), static_cast<long>(result), buf);
  }
  }
}

void testNewYork()
{
  TimeZone tz("/usr/share/zoneinfo/America/New_York");
  TestCase cases[] =
  {

    { "2006-03-07 00:00:00", "2006-03-06 19:00:00-0500(EST)", false },
    { "2006-04-02 06:59:59", "2006-04-02 01:59:59-0500(EST)", false },
    { "2006-04-02 07:00:00", "2006-04-02 03:00:00-0400(EDT)", true  },
    { "2006-05-01 00:00:00", "2006-04-30 20:00:00-0400(EDT)", true  },
    { "2006-05-02 01:00:00", "2006-05-01 21:00:00-0400(EDT)", true  },
    { "2006-10-21 05:00:00", "2006-10-21 01:00:00-0400(EDT)", true  },
    { "2006-10-29 05:59:59", "2006-10-29 01:59:59-0400(EDT)", true  },
    { "2006-10-29 06:00:00", "2006-10-29 01:00:00-0500(EST)", false },
    { "2006-10-29 06:59:59", "2006-10-29 01:59:59-0500(EST)", false },
    { "2006-12-31 06:00:00", "2006-12-31 01:00:00-0500(EST)", false },
    { "2007-01-01 00:00:00", "2006-12-31 19:00:00-0500(EST)", false },

    { "2007-03-07 00:00:00", "2007-03-06 19:00:00-0500(EST)", false },
    { "2007-03-11 06:59:59", "2007-03-11 01:59:59-0500(EST)", false },
    { "2007-03-11 07:00:00", "2007-03-11 03:00:00-0400(EDT)", true  },
    { "2007-05-01 00:00:00", "2007-04-30 20:00:00-0400(EDT)", true  },
    { "2007-05-02 01:00:00", "2007-05-01 21:00:00-0400(EDT)", true  },
    { "2007-10-31 05:00:00", "2007-10-31 01:00:00-0400(EDT)", true  },
    { "2007-11-04 05:59:59", "2007-11-04 01:59:59-0400(EDT)", true  },
    { "2007-11-04 06:00:00", "2007-11-04 01:00:00-0500(EST)", false },
    { "2007-11-04 06:59:59", "2007-11-04 01:59:59-0500(EST)", false },
    { "2007-12-31 06:00:00", "2007-12-31 01:00:00-0500(EST)", false },
    { "2008-01-01 00:00:00", "2007-12-31 19:00:00-0500(EST)", false },

    { "2009-03-07 00:00:00", "2009-03-06 19:00:00-0500(EST)", false },
    { "2009-03-08 06:59:59", "2009-03-08 01:59:59-0500(EST)", false },
    { "2009-03-08 07:00:00", "2009-03-08 03:00:00-0400(EDT)", true  },
    { "2009-05-01 00:00:00", "2009-04-30 20:00:00-0400(EDT)", true  },
    { "2009-05-02 01:00:00", "2009-05-01 21:00:00-0400(EDT)", true  },
    { "2009-10-31 05:00:00", "2009-10-31 01:00:00-0400(EDT)", true  },
    { "2009-11-01 05:59:59", "2009-11-01 01:59:59-0400(EDT)", true  },
    { "2009-11-01 06:00:00", "2009-11-01 01:00:00-0500(EST)", false },
    { "2009-11-01 06:59:59", "2009-11-01 01:59:59-0500(EST)", false },
    { "2009-12-31 06:00:00", "2009-12-31 01:00:00-0500(EST)", false },
    { "2010-01-01 00:00:00", "2009-12-31 19:00:00-0500(EST)", false },

    { "2010-03-13 00:00:00", "2010-03-12 19:00:00-0500(EST)", false },
    { "2010-03-14 06:59:59", "2010-03-14 01:59:59-0500(EST)", false },
    { "2010-03-14 07:00:00", "2010-03-14 03:00:00-0400(EDT)", true  },
    { "2010-05-01 00:00:00", "2010-04-30 20:00:00-0400(EDT)", true  },
    { "2010-05-02 01:00:00", "2010-05-01 21:00:00-0400(EDT)", true  },
    { "2010-11-06 05:00:00", "2010-11-06 01:00:00-0400(EDT)", true  },
    { "2010-11-07 05:59:59", "2010-11-07 01:59:59-0400(EDT)", true  },
    { "2010-11-07 06:00:00", "2010-11-07 01:00:00-0500(EST)", false },
    { "2010-11-07 06:59:59", "2010-11-07 01:59:59-0500(EST)", false },
    { "2010-12-31 06:00:00", "2010-12-31 01:00:00-0500(EST)", false },
    { "2011-01-01 00:00:00", "2010-12-31 19:00:00-0500(EST)", false },

    { "2011-03-01 00:00:00", "2011-02-28 19:00:00-0500(EST)", false },
    { "2011-03-13 06:59:59", "2011-03-13 01:59:59-0500(EST)", false },
    { "2011-03-13 07:00:00", "2011-03-13 03:00:00-0400(EDT)", true  },
    { "2011-05-01 00:00:00", "2011-04-30 20:00:00-0400(EDT)", true  },
    { "2011-05-02 01:00:00", "2011-05-01 21:00:00-0400(EDT)", true  },
    { "2011-11-06 05:59:59", "2011-11-06 01:59:59-0400(EDT)", true  },
    { "2011-11-06 06:00:00", "2011-11-06 01:00:00-0500(EST)", false },
    { "2011-11-06 06:59:59", "2011-11-06 01:59:59-0500(EST)", false },
    { "2011-12-31 06:00:00", "2011-12-31 01:00:00-0500(EST)", false },
    { "2012-01-01 00:00:00", "2011-12-31 19:00:00-0500(EST)", false },

  };

  for (const auto& c : cases)
  {
    test(tz, c);
  }
}

void testLondon()
{
  TimeZone tz("/usr/share/zoneinfo/Europe/London");
  TestCase cases[] =
  {

    { "2011-03-26 00:00:00", "2011-03-26 00:00:00+0000(GMT)", false },
    { "2011-03-27 00:59:59", "2011-03-27 00:59:59+0000(GMT)", false },
    { "2011-03-27 01:00:00", "2011-03-27 02:00:00+0100(BST)", true },
    { "2011-10-30 00:59:59", "2011-10-30 01:59:59+0100(BST)", true },
    { "2011-10-30 01:00:00", "2011-10-30 01:00:00+0000(GMT)", false },
    { "2012-10-30 01:59:59", "2012-10-30 01:59:59+0000(GMT)", false },
    { "2011-12-31 22:00:00", "2011-12-31 22:00:00+0000(GMT)", false },
    { "2012-01-01 00:00:00", "2012-01-01 00:00:00+0000(GMT)", false },

    { "2012-03-24 00:00:00", "2012-03-24 00:00:00+0000(GMT)", false },
    { "2012-03-25 00:59:59", "2012-03-25 00:59:59+0000(GMT)", false },
    { "2012-03-25 01:00:00", "2012-03-25 02:00:00+0100(BST)", true },
    { "2012-10-28 00:59:59", "2012-10-28 01:59:59+0100(BST)", true },
    { "2012-10-28 01:00:00", "2012-10-28 01:00:00+0000(GMT)", false },
    { "2012-10-28 01:59:59", "2012-10-28 01:59:59+0000(GMT)", false },
    { "2012-12-31 22:00:00", "2012-12-31 22:00:00+0000(GMT)", false },
    { "2013-01-01 00:00:00", "2013-01-01 00:00:00+0000(GMT)", false },

  };

  for (const auto& c : cases)
  {
    test(tz, c);
  }
}

void testHongKong()
{
  TimeZone tz("/usr/share/zoneinfo/Asia/Hong_Kong");
  TestCase cases[] =
  {

    { "2011-04-03 00:00:00", "2011-04-03 08:00:00+0800(HKT)", false},

  };

  for (const auto& c : cases)
  {
    test(tz, c);
  }
}

void testSydney()
{
  TimeZone tz("/usr/share/zoneinfo/Australia/Sydney");
  TestCase cases[] =
  {

    { "2011-01-01 00:00:00", "2011-01-01 11:00:00+1100(EST)", true },
    { "2011-04-02 15:59:59", "2011-04-03 02:59:59+1100(EST)", true },
    { "2011-04-02 16:00:00", "2011-04-03 02:00:00+1000(EST)", false },
    { "2011-04-02 16:59:59", "2011-04-03 02:59:59+1000(EST)", false },
    { "2011-05-02 01:00:00", "2011-05-02 11:00:00+1000(EST)", false },
    { "2011-10-01 15:59:59", "2011-10-02 01:59:59+1000(EST)", false },
    { "2011-10-01 16:00:00", "2011-10-02 03:00:00+1100(EST)", true },
    { "2011-12-31 22:00:00", "2012-01-01 09:00:00+1100(EST)", true },

  };

  for (const auto& c : cases)
  {
    test(tz, c);
  }
}

void testUtc()
{
  const int kRange = 100*1000*1000;
  for (time_t t = -kRange; t <= kRange; t += 11)
  {
    struct tm* t1 = gmtime(&t);
    struct tm t2 = TimeZone::toUtcTime(t, true);
    char buf1[80], buf2[80];
    strftime(buf1, sizeof buf1, "%F %T %u %j", t1);
    strftime(buf2, sizeof buf2, "%F %T %u %j", &t2);
    if (strcmp(buf1, buf2) != 0)
    {
      printf("'%s' != '%s'\n", buf1, buf2);
      assert(0);
    }
    time_t t3 = TimeZone::fromUtcTime(t2);
    if (t != t3)
    {
      printf("%ld != %ld\n", static_cast<long>(t), static_cast<long>(t3));
      assert(0);
    }
  }
}

void testFixedTimezone()
{
  TimeZone tz(8*3600, "CST");
  TestCase cases[] =
  {

    { "2014-04-03 00:00:00", "2014-04-03 08:00:00+0800(CST)", false},

  };

  for (const auto& c : cases)
  {
    test(tz, c);
  }
}

int main()
{
  testNewYork();
  testLondon();
  testSydney();
  testHongKong();
  testFixedTimezone();
  testUtc();
}
