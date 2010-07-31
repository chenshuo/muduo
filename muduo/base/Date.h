#ifndef MUDUO_BASE_DATE_H
#define MUDUO_BASE_DATE_H

#include <muduo/base/copyable.h>
#include <muduo/base/Types.h>

namespace muduo
{

class Date : public muduo::copyable
{
 public:
  ///
  /// Constucts an invalid Date.
  ///
  Date();
  Date(int year, int month, int day);
  string toString() const;

  int julianDayNumber() const { return julianDayNumber_; }

 private:
  int julianDayNumber_;
};

}
#endif  // MUDUO_BASE_DATE_H
