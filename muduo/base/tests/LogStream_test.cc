#include <muduo/base/LogStream.h>

#include <limits>
#include <stdint.h>

//#define BOOST_TEST_MODULE LogStreamTest
#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

using muduo::string;

BOOST_AUTO_TEST_CASE(testLogStreamBooleans)
{
  muduo::LogStream os;
  const muduo::LogStream::Buffer& buf = os.buffer();
  BOOST_CHECK_EQUAL(buf.asString(), string(""));
  os << true;
  BOOST_CHECK_EQUAL(buf.asString(), string("1"));
  os << '\n';
  BOOST_CHECK_EQUAL(buf.asString(), string("1\n"));
  os << false;
  BOOST_CHECK_EQUAL(buf.asString(), string("1\n0"));
}

BOOST_AUTO_TEST_CASE(testLogStreamIntegers)
{
  muduo::LogStream os;
  const muduo::LogStream::Buffer& buf = os.buffer();
  BOOST_CHECK_EQUAL(buf.asString(), string(""));
  os << 1;
  BOOST_CHECK_EQUAL(buf.asString(), string("1"));
  os << 0;
  BOOST_CHECK_EQUAL(buf.asString(), string("10"));
  os << -1;
  BOOST_CHECK_EQUAL(buf.asString(), string("10-1"));
  os.resetBuffer();

  os << 0 << " " << 123 << 'x' << 0x64;
  BOOST_CHECK_EQUAL(buf.asString(), string("0 123x100"));
}

BOOST_AUTO_TEST_CASE(testLogStreamIntegerLimits)
{
  muduo::LogStream os;
  const muduo::LogStream::Buffer& buf = os.buffer();
  os << -2147483647;
  BOOST_CHECK_EQUAL(buf.asString(), string("-2147483647"));
  os << static_cast<int>(-2147483647 - 1);
  BOOST_CHECK_EQUAL(buf.asString(), string("-2147483647-2147483648"));
  os << ' ';
  os << 2147483647;
  BOOST_CHECK_EQUAL(buf.asString(), string("-2147483647-2147483648 2147483647"));
  os.resetBuffer();

  os << std::numeric_limits<int16_t>::min();
  BOOST_CHECK_EQUAL(buf.asString(), string("-32768"));
  os.resetBuffer();

  os << std::numeric_limits<int16_t>::max();
  BOOST_CHECK_EQUAL(buf.asString(), string("32767"));
  os.resetBuffer();

  os << std::numeric_limits<uint16_t>::min();
  BOOST_CHECK_EQUAL(buf.asString(), string("0"));
  os.resetBuffer();

  os << std::numeric_limits<uint16_t>::max();
  BOOST_CHECK_EQUAL(buf.asString(), string("65535"));
  os.resetBuffer();

  os << std::numeric_limits<int32_t>::min();
  BOOST_CHECK_EQUAL(buf.asString(), string("-2147483648"));
  os.resetBuffer();

  os << std::numeric_limits<int32_t>::max();
  BOOST_CHECK_EQUAL(buf.asString(), string("2147483647"));
  os.resetBuffer();

  os << std::numeric_limits<uint32_t>::min();
  BOOST_CHECK_EQUAL(buf.asString(), string("0"));
  os.resetBuffer();

  os << std::numeric_limits<uint32_t>::max();
  BOOST_CHECK_EQUAL(buf.asString(), string("4294967295"));
  os.resetBuffer();

  os << std::numeric_limits<int64_t>::min();
  BOOST_CHECK_EQUAL(buf.asString(), string("-9223372036854775808"));
  os.resetBuffer();

  os << std::numeric_limits<int64_t>::max();
  BOOST_CHECK_EQUAL(buf.asString(), string("9223372036854775807"));
  os.resetBuffer();

  os << std::numeric_limits<uint64_t>::min();
  BOOST_CHECK_EQUAL(buf.asString(), string("0"));
  os.resetBuffer();

  os << std::numeric_limits<uint64_t>::max();
  BOOST_CHECK_EQUAL(buf.asString(), string("18446744073709551615"));
  os.resetBuffer();

  int16_t a = 0;
  int32_t b = 0;
  int64_t c = 0;
  os << a;
  os << b;
  os << c;
  BOOST_CHECK_EQUAL(buf.asString(), string("000"));
}

BOOST_AUTO_TEST_CASE(testLogStreamFloats)
{
  muduo::LogStream os;
  const muduo::LogStream::Buffer& buf = os.buffer();

  os << 0.0;
  BOOST_CHECK_EQUAL(buf.asString(), string("0"));
  os.resetBuffer();

  os << 1.0;
  BOOST_CHECK_EQUAL(buf.asString(), string("1"));
  os.resetBuffer();

  os << 0.1;
  BOOST_CHECK_EQUAL(buf.asString(), string("0.1"));
  os.resetBuffer();

  os << 0.05;
  BOOST_CHECK_EQUAL(buf.asString(), string("0.05"));
  os.resetBuffer();

  os << 0.15;
  BOOST_CHECK_EQUAL(buf.asString(), string("0.15"));
  os.resetBuffer();

  double a = 0.1;
  os << a;
  BOOST_CHECK_EQUAL(buf.asString(), string("0.1"));
  os.resetBuffer();

  double b = 0.05;
  os << b;
  BOOST_CHECK_EQUAL(buf.asString(), string("0.05"));
  os.resetBuffer();

  double c = 0.15;
  os << c;
  BOOST_CHECK_EQUAL(buf.asString(), string("0.15"));
  os.resetBuffer();

  os << a+b;
  BOOST_CHECK_EQUAL(buf.asString(), string("0.15"));
  os.resetBuffer();

  BOOST_CHECK(a+b != c);

  os << 1.23456789;
  BOOST_CHECK_EQUAL(buf.asString(), string("1.23456789"));
  os.resetBuffer();

  os << 1.234567;
  BOOST_CHECK_EQUAL(buf.asString(), string("1.234567"));
  os.resetBuffer();

  os << -123.456;
  BOOST_CHECK_EQUAL(buf.asString(), string("-123.456"));
  os.resetBuffer();
}

BOOST_AUTO_TEST_CASE(testLogStreamVoid)
{
  muduo::LogStream os;
  const muduo::LogStream::Buffer& buf = os.buffer();

  os << static_cast<void*>(0);
  BOOST_CHECK_EQUAL(buf.asString(), string("0x0"));
  os.resetBuffer();

  os << reinterpret_cast<void*>(8888);
  BOOST_CHECK_EQUAL(buf.asString(), string("0x22B8"));
  os.resetBuffer();
}

BOOST_AUTO_TEST_CASE(testLogStreamStrings)
{
  muduo::LogStream os;
  const muduo::LogStream::Buffer& buf = os.buffer();

  os << "Hello ";
  BOOST_CHECK_EQUAL(buf.asString(), string("Hello "));

  string chenshuo = "Shuo Chen";
  os << chenshuo;
  BOOST_CHECK_EQUAL(buf.asString(), string("Hello Shuo Chen"));
}

BOOST_AUTO_TEST_CASE(testLogStreamFmts)
{
  muduo::LogStream os;
  const muduo::LogStream::Buffer& buf = os.buffer();

  os << muduo::Fmt("%4d", 1);
  BOOST_CHECK_EQUAL(buf.asString(), string("   1"));
  os.resetBuffer();

  os << muduo::Fmt("%4.2f", 1.2);
  BOOST_CHECK_EQUAL(buf.asString(), string("1.20"));
  os.resetBuffer();

  os << muduo::Fmt("%4.2f", 1.2) << muduo::Fmt("%4d", 43);
  BOOST_CHECK_EQUAL(buf.asString(), string("1.20  43"));
  os.resetBuffer();
}

BOOST_AUTO_TEST_CASE(testLogStreamLong)
{
  muduo::LogStream os;
  const muduo::LogStream::Buffer& buf = os.buffer();
  for (int i = 0; i < 399; ++i)
  {
    os << "123456789 ";
    BOOST_CHECK_EQUAL(buf.length(), 10*(i+1));
    BOOST_CHECK_EQUAL(buf.avail(), 4000 - 10*(i+1));
  }

  os << "abcdefghi ";
  BOOST_CHECK_EQUAL(buf.length(), 3990);
  BOOST_CHECK_EQUAL(buf.avail(), 10);

  os << "abcdefghi";
  BOOST_CHECK_EQUAL(buf.length(), 3999);
  BOOST_CHECK_EQUAL(buf.avail(), 1);
}
