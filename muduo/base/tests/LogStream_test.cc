#include <muduo/base/LogStream.h>

#include <limits>
#include <stdint.h>

#include <gtest/gtest.h>

using muduo::string;

TEST(testLogStreamBooleans, boolTest)
{
  muduo::LogStream os;
  const muduo::LogStream::Buffer& buf = os.buffer();
  EXPECT_EQ(buf.toString(), string(""));
  os << true;
  EXPECT_EQ(buf.toString(), string("1"));
  os << '\n';
  EXPECT_EQ(buf.toString(), string("1\n"));
  os << false;
  EXPECT_EQ(buf.toString(), string("1\n0"));
}

TEST(testLogStreamIntegers, intTest)
{
  muduo::LogStream os;
  const muduo::LogStream::Buffer& buf = os.buffer();
  EXPECT_EQ(buf.toString(), string(""));
  os << 1;
  EXPECT_EQ(buf.toString(), string("1"));
  os << 0;
  EXPECT_EQ(buf.toString(), string("10"));
  os << -1;
  EXPECT_EQ(buf.toString(), string("10-1"));
  os.resetBuffer();

  os << 0 << " " << 123 << 'x' << 0x64;
  EXPECT_EQ(buf.toString(), string("0 123x100"));
}

TEST(testLogStreamIntegerLimits, intLimitsTest)
{
  muduo::LogStream os;
  const muduo::LogStream::Buffer& buf = os.buffer();
  os << -2147483647;
  EXPECT_EQ(buf.toString(), string("-2147483647"));
  os << static_cast<int>(-2147483647 - 1);
  EXPECT_EQ(buf.toString(), string("-2147483647-2147483648"));
  os << ' ';
  os << 2147483647;
  EXPECT_EQ(buf.toString(), string("-2147483647-2147483648 2147483647"));
  os.resetBuffer();

  os << std::numeric_limits<int16_t>::min();
  EXPECT_EQ(buf.toString(), string("-32768"));
  os.resetBuffer();

  os << std::numeric_limits<int16_t>::max();
  EXPECT_EQ(buf.toString(), string("32767"));
  os.resetBuffer();

  os << std::numeric_limits<uint16_t>::min();
  EXPECT_EQ(buf.toString(), string("0"));
  os.resetBuffer();

  os << std::numeric_limits<uint16_t>::max();
  EXPECT_EQ(buf.toString(), string("65535"));
  os.resetBuffer();

  os << std::numeric_limits<int32_t>::min();
  EXPECT_EQ(buf.toString(), string("-2147483648"));
  os.resetBuffer();

  os << std::numeric_limits<int32_t>::max();
  EXPECT_EQ(buf.toString(), string("2147483647"));
  os.resetBuffer();

  os << std::numeric_limits<uint32_t>::min();
  EXPECT_EQ(buf.toString(), string("0"));
  os.resetBuffer();

  os << std::numeric_limits<uint32_t>::max();
  EXPECT_EQ(buf.toString(), string("4294967295"));
  os.resetBuffer();

  os << std::numeric_limits<int64_t>::min();
  EXPECT_EQ(buf.toString(), string("-9223372036854775808"));
  os.resetBuffer();

  os << std::numeric_limits<int64_t>::max();
  EXPECT_EQ(buf.toString(), string("9223372036854775807"));
  os.resetBuffer();

  os << std::numeric_limits<uint64_t>::min();
  EXPECT_EQ(buf.toString(), string("0"));
  os.resetBuffer();

  os << std::numeric_limits<uint64_t>::max();
  EXPECT_EQ(buf.toString(), string("18446744073709551615"));
  os.resetBuffer();

  int16_t a = 0;
  int32_t b = 0;
  int64_t c = 0;
  os << a;
  os << b;
  os << c;
  EXPECT_EQ(buf.toString(), string("000"));
}

TEST(testLogStreamFloats, floatTest)
{
  muduo::LogStream os;
  const muduo::LogStream::Buffer& buf = os.buffer();

  os << 0.0;
  EXPECT_EQ(buf.toString(), string("0"));
  os.resetBuffer();

  os << 1.0;
  EXPECT_EQ(buf.toString(), string("1"));
  os.resetBuffer();

  os << 0.1;
  EXPECT_EQ(buf.toString(), string("0.1"));
  os.resetBuffer();

  os << 0.05;
  EXPECT_EQ(buf.toString(), string("0.05"));
  os.resetBuffer();

  os << 0.15;
  EXPECT_EQ(buf.toString(), string("0.15"));
  os.resetBuffer();

  double a = 0.1;
  os << a;
  EXPECT_EQ(buf.toString(), string("0.1"));
  os.resetBuffer();

  double b = 0.05;
  os << b;
  EXPECT_EQ(buf.toString(), string("0.05"));
  os.resetBuffer();

  double c = 0.15;
  os << c;
  EXPECT_EQ(buf.toString(), string("0.15"));
  os.resetBuffer();

  os << a+b;
  EXPECT_EQ(buf.toString(), string("0.15"));
  os.resetBuffer();

  GTEST_CHECK_(a+b != c);

  os << 1.23456789;
  EXPECT_EQ(buf.toString(), string("1.23456789"));
  os.resetBuffer();

  os << 1.234567;
  EXPECT_EQ(buf.toString(), string("1.234567"));
  os.resetBuffer();

  os << -123.456;
  EXPECT_EQ(buf.toString(), string("-123.456"));
  os.resetBuffer();
}

TEST(testLogStreamVoid, voidTest)
{
  muduo::LogStream os;
  const muduo::LogStream::Buffer& buf = os.buffer();

  os << static_cast<void*>(0);
  EXPECT_EQ(buf.toString(), string("0x0"));
  os.resetBuffer();

  os << reinterpret_cast<void*>(8888);
  EXPECT_EQ(buf.toString(), string("0x22B8"));
  os.resetBuffer();
}

TEST(testLogStreamStrings, stringTest)
{
  muduo::LogStream os;
  const muduo::LogStream::Buffer& buf = os.buffer();

  os << "Hello ";
  EXPECT_EQ(buf.toString(), string("Hello "));

  string chenshuo = "Shuo Chen";
  os << chenshuo;
  EXPECT_EQ(buf.toString(), string("Hello Shuo Chen"));
}

TEST(testLogStreamFmts, fmtTest)
{
  muduo::LogStream os;
  const muduo::LogStream::Buffer& buf = os.buffer();

  os << muduo::Fmt("%4d", 1);
  EXPECT_EQ(buf.toString(), string("   1"));
  os.resetBuffer();

  os << muduo::Fmt("%4.2f", 1.2);
  EXPECT_EQ(buf.toString(), string("1.20"));
  os.resetBuffer();

  os << muduo::Fmt("%4.2f", 1.2) << muduo::Fmt("%4d", 43);
  EXPECT_EQ(buf.toString(), string("1.20  43"));
  os.resetBuffer();
}

TEST(testLogStreamLong, longTest)
{
  muduo::LogStream os;
  const muduo::LogStream::Buffer& buf = os.buffer();
  for (int i = 0; i < 399; ++i)
  {
    os << "123456789 ";
    EXPECT_EQ(buf.length(), 10*(i+1));
    EXPECT_EQ(buf.avail(), 4000 - 10*(i+1));
  }

  os << "abcdefghi ";
  EXPECT_EQ(buf.length(), 3990);
  EXPECT_EQ(buf.avail(), 10);

  os << "abcdefghi";
  EXPECT_EQ(buf.length(), 3999);
  EXPECT_EQ(buf.avail(), 1);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

