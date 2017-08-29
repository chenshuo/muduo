#ifndef MUDUO_BASE_STRINGPIECE_H
#define MUDUO_BASE_STRINGPIECE_H

#include <string_view>

#include <muduo/base/Types.h>

namespace muduo
{

// For passing C-style string argument to a function.
class StringArg // copyable
{
 public:
  StringArg(const char* str)
    : str_(str)
  { }

  StringArg(const string& str)
    : str_(str.c_str())
  { }

  const char* c_str() const { return str_; }

 private:
  const char* str_;
};

typedef std::string_view StringPiece;

}  // namespace muduo

#endif  // MUDUO_BASE_STRINGPIECE_H
