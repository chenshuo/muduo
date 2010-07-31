#include <muduo/base/Date.h>

using namespace muduo;

Date::Date()
  : julianDayNumber_(0)
{
}

Date::Date(int year, int month, int day)
  : julianDayNumber_(0)
{
}

string Date::toString() const
{
  return "";
}

