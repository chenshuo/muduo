#include "muduo/base/LogStream.h"

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
  BOOST_CHECK_EQUAL(buf.toString(), string(""));
  os << true;
  BOOST_CHECK_EQUAL(buf.toString(), string("1"));
  os << '\n';
  BOOST_CHECK_EQUAL(buf.toString(), string("1\n"));
  os << false;
  BOOST_CHECK_EQUAL(buf.toString(), string("1\n0"));
}

BOOST_AUTO_TEST_CASE(testLogStreamIntegers)
{
  muduo::LogStream os;
  const muduo::LogStream::Buffer& buf = os.buffer();
  BOOST_CHECK_EQUAL(buf.toString(), string(""));
  os << 1;
  BOOST_CHECK_EQUAL(buf.toString(), string("1"));
  os << 0;
  BOOST_CHECK_EQUAL(buf.toString(), string("10"));
  os << -1;
  BOOST_CHECK_EQUAL(buf.toString(), string("10-1"));
  os.resetBuffer();

  os << 0 << " " << 123 << 'x' << 0x64;
  BOOST_CHECK_EQUAL(buf.toString(), string("0 123x100"));
}

BOOST_AUTO_TEST_CASE(testLogStreamIntegerLimits)
{
  muduo::LogStream os;
  const muduo::LogStream::Buffer& buf = os.buffer();
  os << -2147483647;
  BOOST_CHECK_EQUAL(buf.toString(), string("-2147483647"));
  os << static_cast<int>(-2147483647 - 1);
  BOOST_CHECK_EQUAL(buf.toString(), string("-2147483647-2147483648"));
  os << ' ';
  os << 2147483647;
  BOOST_CHECK_EQUAL(buf.toString(), string("-2147483647-2147483648 2147483647"));
  os.resetBuffer();

  os << std::numeric_limits<int16_t>::min();
  BOOST_CHECK_EQUAL(buf.toString(), string("-32768"));
  os.resetBuffer();

  os << std::numeric_limits<int16_t>::max();
  BOOST_CHECK_EQUAL(buf.toString(), string("32767"));
  os.resetBuffer();

  os << std::numeric_limits<uint16_t>::min();
  BOOST_CHECK_EQUAL(buf.toString(), string("0"));
  os.resetBuffer();

  os << std::numeric_limits<uint16_t>::max();
  BOOST_CHECK_EQUAL(buf.toString(), string("65535"));
  os.resetBuffer();

  os << std::numeric_limits<int32_t>::min();
  BOOST_CHECK_EQUAL(buf.toString(), string("-2147483648"));
  os.resetBuffer();

  os << std::numeric_limits<int32_t>::max();
  BOOST_CHECK_EQUAL(buf.toString(), string("2147483647"));
  os.resetBuffer();

  os << std::numeric_limits<uint32_t>::min();
  BOOST_CHECK_EQUAL(buf.toString(), string("0"));
  os.resetBuffer();

  os << std::numeric_limits<uint32_t>::max();
  BOOST_CHECK_EQUAL(buf.toString(), string("4294967295"));
  os.resetBuffer();

  os << std::numeric_limits<int64_t>::min();
  BOOST_CHECK_EQUAL(buf.toString(), string("-9223372036854775808"));
  os.resetBuffer();

  os << std::numeric_limits<int64_t>::max();
  BOOST_CHECK_EQUAL(buf.toString(), string("9223372036854775807"));
  os.resetBuffer();

  os << std::numeric_limits<uint64_t>::min();
  BOOST_CHECK_EQUAL(buf.toString(), string("0"));
  os.resetBuffer();

  os << std::numeric_limits<uint64_t>::max();
  BOOST_CHECK_EQUAL(buf.toString(), string("18446744073709551615"));
  os.resetBuffer();

  int16_t a = 0;
  int32_t b = 0;
  int64_t c = 0;
  os << a;
  os << b;
  os << c;
  BOOST_CHECK_EQUAL(buf.toString(), string("000"));
}

BOOST_AUTO_TEST_CASE(testLogStreamFloats)
{
  muduo::LogStream os;
  const muduo::LogStream::Buffer& buf = os.buffer();

  os << 0.0;
  BOOST_CHECK_EQUAL(buf.toString(), string("0"));
  os.resetBuffer();

  os << 1.0;
  BOOST_CHECK_EQUAL(buf.toString(), string("1"));
  os.resetBuffer();

  os << 0.1;
  BOOST_CHECK_EQUAL(buf.toString(), string("0.1"));
  os.resetBuffer();

  os << 0.05;
  BOOST_CHECK_EQUAL(buf.toString(), string("0.05"));
  os.resetBuffer();

  os << 0.15;
  BOOST_CHECK_EQUAL(buf.toString(), string("0.15"));
  os.resetBuffer();

  double a = 0.1;
  os << a;
  BOOST_CHECK_EQUAL(buf.toString(), string("0.1"));
  os.resetBuffer();

  double b = 0.05;
  os << b;
  BOOST_CHECK_EQUAL(buf.toString(), string("0.05"));
  os.resetBuffer();

  double c = 0.15;
  os << c;
  BOOST_CHECK_EQUAL(buf.toString(), string("0.15"));
  os.resetBuffer();

  os << a+b;
  BOOST_CHECK_EQUAL(buf.toString(), string("0.15"));
  os.resetBuffer();

  BOOST_CHECK(a+b != c);

  os << 1.23456789;
  BOOST_CHECK_EQUAL(buf.toString(), string("1.23456789"));
  os.resetBuffer();

  os << 1.234567;
  BOOST_CHECK_EQUAL(buf.toString(), string("1.234567"));
  os.resetBuffer();

  os << -123.456;
  BOOST_CHECK_EQUAL(buf.toString(), string("-123.456"));
  os.resetBuffer();
}

BOOST_AUTO_TEST_CASE(testLogStreamVoid)
{
  muduo::LogStream os;
  const muduo::LogStream::Buffer& buf = os.buffer();

  os << static_cast<void*>(0);
  BOOST_CHECK_EQUAL(buf.toString(), string("0x0"));
  os.resetBuffer();

  os << reinterpret_cast<void*>(8888);
  BOOST_CHECK_EQUAL(buf.toString(), string("0x22B8"));
  os.resetBuffer();
}

BOOST_AUTO_TEST_CASE(testLogStreamStrings)
{
  muduo::LogStream os;
  const muduo::LogStream::Buffer& buf = os.buffer();

  os << "Hello ";
  BOOST_CHECK_EQUAL(buf.toString(), string("Hello "));

  string chenshuo = "Shuo Chen";
  os << chenshuo;
  BOOST_CHECK_EQUAL(buf.toString(), string("Hello Shuo Chen"));
}

BOOST_AUTO_TEST_CASE(testLogStreamFmts)
{
  muduo::LogStream os;
  const muduo::LogStream::Buffer& buf = os.buffer();

  os << muduo::Fmt("%4d", 1);
  BOOST_CHECK_EQUAL(buf.toString(), string("   1"));
  os.resetBuffer();

  os << muduo::Fmt("%4.2f", 1.2);
  BOOST_CHECK_EQUAL(buf.toString(), string("1.20"));
  os.resetBuffer();

  os << muduo::Fmt("%4.2f", 1.2) << muduo::Fmt("%4d", 43);
  BOOST_CHECK_EQUAL(buf.toString(), string("1.20  43"));
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

BOOST_AUTO_TEST_CASE(testFormatSI)
{
  BOOST_CHECK_EQUAL(muduo::formatSI(0), string("0"));
  BOOST_CHECK_EQUAL(muduo::formatSI(999), string("999"));
  BOOST_CHECK_EQUAL(muduo::formatSI(1000), string("1.00k"));
  BOOST_CHECK_EQUAL(muduo::formatSI(9990), string("9.99k"));
  BOOST_CHECK_EQUAL(muduo::formatSI(9994), string("9.99k"));
  BOOST_CHECK_EQUAL(muduo::formatSI(9995), string("10.0k"));
  BOOST_CHECK_EQUAL(muduo::formatSI(10000), string("10.0k"));
  BOOST_CHECK_EQUAL(muduo::formatSI(10049), string("10.0k"));
  BOOST_CHECK_EQUAL(muduo::formatSI(10050), string("10.1k"));
  BOOST_CHECK_EQUAL(muduo::formatSI(99900), string("99.9k"));
  BOOST_CHECK_EQUAL(muduo::formatSI(99949), string("99.9k"));
  BOOST_CHECK_EQUAL(muduo::formatSI(99950), string("100k"));
  BOOST_CHECK_EQUAL(muduo::formatSI(100499), string("100k"));
  // FIXME:
  // BOOST_CHECK_EQUAL(muduo::formatSI(100500), string("101k"));
  BOOST_CHECK_EQUAL(muduo::formatSI(100501), string("101k"));
  BOOST_CHECK_EQUAL(muduo::formatSI(999499), string("999k"));
  BOOST_CHECK_EQUAL(muduo::formatSI(999500), string("1.00M"));
  BOOST_CHECK_EQUAL(muduo::formatSI(1004999), string("1.00M"));
  // BOOST_CHECK_EQUAL(muduo::formatSI(1005000), string("1.01M"));
  BOOST_CHECK_EQUAL(muduo::formatSI(1005001), string("1.01M"));
  BOOST_CHECK_EQUAL(muduo::formatSI(INT64_MAX), string("9.22E"));
}

BOOST_AUTO_TEST_CASE(testFormatIEC)
{
  BOOST_CHECK_EQUAL(muduo::formatIEC(0), string("0"));
  BOOST_CHECK_EQUAL(muduo::formatIEC(1023), string("1023"));
  BOOST_CHECK_EQUAL(muduo::formatIEC(1024), string("1.00Ki"));
  BOOST_CHECK_EQUAL(muduo::formatIEC(10234), string("9.99Ki"));
  BOOST_CHECK_EQUAL(muduo::formatIEC(10235), string("10.0Ki"));
  BOOST_CHECK_EQUAL(muduo::formatIEC(10240), string("10.0Ki"));
  BOOST_CHECK_EQUAL(muduo::formatIEC(10291), string("10.0Ki"));
  BOOST_CHECK_EQUAL(muduo::formatIEC(10292), string("10.1Ki"));
  BOOST_CHECK_EQUAL(muduo::formatIEC(102348), string("99.9Ki"));
  BOOST_CHECK_EQUAL(muduo::formatIEC(102349), string("100Ki"));
  BOOST_CHECK_EQUAL(muduo::formatIEC(102912), string("100Ki"));
  BOOST_CHECK_EQUAL(muduo::formatIEC(102913), string("101Ki"));
  BOOST_CHECK_EQUAL(muduo::formatIEC(1022976), string("999Ki"));
  BOOST_CHECK_EQUAL(muduo::formatIEC(1047552), string("1023Ki"));
  BOOST_CHECK_EQUAL(muduo::formatIEC(1047961), string("1023Ki"));
  BOOST_CHECK_EQUAL(muduo::formatIEC(1048063), string("1023Ki"));
  BOOST_CHECK_EQUAL(muduo::formatIEC(1048064), string("1.00Mi"));
  BOOST_CHECK_EQUAL(muduo::formatIEC(1048576), string("1.00Mi"));
  BOOST_CHECK_EQUAL(muduo::formatIEC(10480517), string("9.99Mi"));
  BOOST_CHECK_EQUAL(muduo::formatIEC(10480518), string("10.0Mi"));
  BOOST_CHECK_EQUAL(muduo::formatIEC(INT64_MAX), string("8.00Ei"));
}
