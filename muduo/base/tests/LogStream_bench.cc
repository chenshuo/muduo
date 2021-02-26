#include <muduo/base/LogStream.h>
#include <muduo/base/Timestamp.h>

#include <sstream>
#include <stdio.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

using namespace muduo;

const size_t N = 1000000;

#pragma GCC diagnostic ignored "-Wold-style-cast"

template<typename T>
void benchPrintf(const char* fmt)
{
  char buf[32];
  Timestamp start(Timestamp::now());
  for (size_t i = 0; i < N; ++i)
    snprintf(buf, sizeof buf, fmt, (T)(i));
  Timestamp end(Timestamp::now());

  printf("benchPrintf %f\n", timeDifference(end, start));
}

template<typename T>
void benchStringStream()
{
  Timestamp start(Timestamp::now());
  std::ostringstream os;

  for (size_t i = 0; i < N; ++i)
  {
    os << (T)(i);
    os.seekp(0, std::ios_base::beg);
  }
  Timestamp end(Timestamp::now());

  printf("benchStringStream %f\n", timeDifference(end, start));
}

template<typename T>
void benchLogStream()
{
  Timestamp start(Timestamp::now());
  LogStream os;
  for (size_t i = 0; i < N; ++i)
  {
    os << (T)(i);
    os.resetBuffer();
  }
  Timestamp end(Timestamp::now());

  printf("benchLogStream %f\n", timeDifference(end, start));
}

int main()
{
  benchPrintf<int>("%d");

  puts("int");
  benchPrintf<int>("%d");
  benchStringStream<int>();
  benchLogStream<int>();

  puts("double");
  benchPrintf<double>("%.12g");
  benchStringStream<double>();
  benchLogStream<double>();

  puts("int64_t");
  benchPrintf<int64_t>("%" PRId64);
  benchStringStream<int64_t>();
  benchLogStream<int64_t>();

  puts("void*");
  benchPrintf<void*>("%p");
  benchStringStream<void*>();
  benchLogStream<void*>();

}
