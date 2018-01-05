#ifndef MUDUO_BASE_TIMESTAMP_H
#define MUDUO_BASE_TIMESTAMP_H

#include <muduo/base/copyable.h>
#include <muduo/base/Types.h>

#include <boost/operators.hpp>

namespace muduo
{

/// Time stamp in UTC, 毫秒级
class Timestamp : public muduo::copyable
{
 public:
  Timestamp()
    : mSecondsSinceEpoch_(0)
  {
  }

  explicit Timestamp(int64_t mSeconds)
    : mSecondsSinceEpoch_(mSeconds)
  {
  }

  void swap(Timestamp& that)
  {
    std::swap(mSecondsSinceEpoch_, that.mSecondsSinceEpoch_);
  }

  string toString() const;
  string toFormattedString(bool nouse = true) const;

  bool valid() const { return mSecondsSinceEpoch_ > 0; }

  int64_t microSecondsSinceEpoch() const
  {
    return mSecondsSinceEpoch_ * 1000;
  }

  int64_t milliSecondsSinceEpoch() const
  {
    return mSecondsSinceEpoch_;
  }

  time_t secondsSinceEpoch() const
  {
    return static_cast<time_t>((mSecondsSinceEpoch_ + 500) / kMicroSecondsPerSecond);
  }

  static Timestamp now();
  static Timestamp invalid()
  {
    return Timestamp();
  }

  static Timestamp fromUnixTime(time_t t)
  {
    return fromUnixTime(t, 0);
  }

  static Timestamp fromUnixTime(time_t t, int microseconds)
  {
    return Timestamp(static_cast<int64_t>(t) * kMicroSecondsPerSecond + microseconds);
  }

  static const int kMicroSecondsPerSecond = 1000 * 1000;

public:
  friend bool operator<(Timestamp lhs, Timestamp rhs)
  {
    return lhs.mSecondsSinceEpoch_ < rhs.mSecondsSinceEpoch_;
  }

  friend bool operator>(Timestamp lhs, Timestamp rhs)
  {
    return lhs.mSecondsSinceEpoch_ > rhs.mSecondsSinceEpoch_;
  }

  friend bool operator==(Timestamp lhs, Timestamp rhs)
  {
    return lhs.mSecondsSinceEpoch_ == rhs.mSecondsSinceEpoch_;
  }

  friend int64_t operator-(Timestamp lhs, Timestamp rhs)
  {
    return static_cast<int64_t>(lhs.mSecondsSinceEpoch_ - rhs.mSecondsSinceEpoch_);
  }

  operator int64_t()
  {
    return this->mSecondsSinceEpoch_;
  }

  friend double timeDifference(Timestamp high, Timestamp low)
  {
    int64_t diff = high.microSecondsSinceEpoch() - low.microSecondsSinceEpoch();
    return static_cast<double>(diff) / Timestamp::kMicroSecondsPerSecond;
  }

  friend Timestamp addTime(Timestamp timestamp, double seconds)
  {
    int64_t delta = static_cast<int64_t>(seconds * 1000);
    return Timestamp(timestamp.mSecondsSinceEpoch_ + delta);
  }

 private:
  int64_t mSecondsSinceEpoch_;
};

}
#endif  // MUDUO_BASE_TIMESTAMP_H
