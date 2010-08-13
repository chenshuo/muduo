#ifndef MUDUO_BASE_TIMEZONE_H
#define MUDUO_BASE_TIMEZONE_H

#include <muduo/base/copyable.h>
#include <boost/shared_ptr.hpp>
#include <time.h>

namespace muduo
{

class TimeZone : public muduo::copyable
{
 public:
  explicit TimeZone(const char* zonefile);

  // default copy ctor/assignment/dtor are Okay.

  bool valid() const { return data_; }
  struct tm toLocalTime(time_t secondsSinceEpoch) const;
  time_t fromLocalTime(const struct tm&) const;

  struct Data;

 private:
  boost::shared_ptr<Data> data_;
};

}
#endif  // MUDUO_BASE_TIMEZONE_H
