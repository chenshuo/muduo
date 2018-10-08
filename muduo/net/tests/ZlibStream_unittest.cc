#include <muduo/net/ZlibStream.h>

#include <muduo/base/Logging.h>

#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <stdio.h>

BOOST_AUTO_TEST_CASE(testZlibOutputStream)
{
  muduo::net::Buffer output;
  {
    muduo::net::ZlibOutputStream stream(&output);
    BOOST_CHECK_EQUAL(output.readableBytes(), 0);
  }
  BOOST_CHECK_EQUAL(output.readableBytes(), 8);
}

BOOST_AUTO_TEST_CASE(testZlibOutputStream1)
{
  muduo::net::Buffer output;
  muduo::net::ZlibOutputStream stream(&output);
  BOOST_CHECK_EQUAL(stream.zlibErrorCode(), Z_OK);
  stream.finish();
  BOOST_CHECK_EQUAL(stream.zlibErrorCode(), Z_STREAM_END);
}

BOOST_AUTO_TEST_CASE(testZlibOutputStream2)
{
  muduo::net::Buffer output;
  muduo::net::ZlibOutputStream stream(&output);
  BOOST_CHECK_EQUAL(stream.zlibErrorCode(), Z_OK);
  BOOST_CHECK(stream.write("01234567890123456789012345678901234567890123456789"));
  stream.finish();
  // printf("%zd\n", output.readableBytes());
  BOOST_CHECK_EQUAL(stream.zlibErrorCode(), Z_STREAM_END);
}

BOOST_AUTO_TEST_CASE(testZlibOutputStream3)
{
  muduo::net::Buffer output;
  muduo::net::ZlibOutputStream stream(&output);
  BOOST_CHECK_EQUAL(stream.zlibErrorCode(), Z_OK);
  for (int i = 0; i < 1024*1024; ++i)
  {
    BOOST_CHECK(stream.write("01234567890123456789012345678901234567890123456789"));
  }
  stream.finish();
  // printf("total %zd\n", output.readableBytes());
  BOOST_CHECK_EQUAL(stream.zlibErrorCode(), Z_STREAM_END);
}

BOOST_AUTO_TEST_CASE(testZlibOutputStream4)
{
  muduo::net::Buffer output;
  muduo::net::ZlibOutputStream stream(&output);
  BOOST_CHECK_EQUAL(stream.zlibErrorCode(), Z_OK);
  muduo::string input;
  for (int i = 0; i < 32768; ++i)
  {
    input += "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_-"[rand() % 64];
  }

  for (int i = 0; i < 10; ++i)
  {
    BOOST_CHECK(stream.write(input));
  }
  stream.finish();
  // printf("total %zd\n", output.readableBytes());
  BOOST_CHECK_EQUAL(stream.zlibErrorCode(), Z_STREAM_END);
}

BOOST_AUTO_TEST_CASE(testZlibOutputStream5)
{
  muduo::net::Buffer output;
  muduo::net::ZlibOutputStream stream(&output);
  BOOST_CHECK_EQUAL(stream.zlibErrorCode(), Z_OK);
  muduo::string input(1024*1024, '_');
  for (int i = 0; i < 64; ++i)
  {
    BOOST_CHECK(stream.write(input));
  }
  printf("bufsiz %d\n", stream.internalOutputBufferSize());
  LOG_INFO << "total_in " << stream.inputBytes();
  LOG_INFO << "total_out " << stream.outputBytes();
  stream.finish();
  printf("total %zd\n", output.readableBytes());
  BOOST_CHECK_EQUAL(stream.zlibErrorCode(), Z_STREAM_END);
}

BOOST_AUTO_TEST_CASE(testZlibInputStream)
{
  muduo::net::Buffer input;
  {
    muduo::net::ZlibInputStream stream(&input);
    BOOST_CHECK_EQUAL(input.readableBytes(), 0);
  }
  BOOST_CHECK_EQUAL(input.readableBytes(), 0);
}


BOOST_AUTO_TEST_CASE(testZlibInputStream1)
{
  muduo::net::Buffer input;
  muduo::net::ZlibInputStream stream(&input);
  BOOST_CHECK_EQUAL(stream.zlibErrorCode(), Z_OK);
  stream.finish();
  BOOST_CHECK_EQUAL(stream.zlibErrorCode(), Z_STREAM_END);
}

BOOST_AUTO_TEST_CASE(testZlibInputStream2)
{
  muduo::net::Buffer output;
  muduo::net::ZlibOutputStream stream(&output);
  BOOST_CHECK_EQUAL(stream.zlibErrorCode(), Z_OK);
  BOOST_CHECK(stream.write("01234567890123456789012345678901234567890123456789"));
  stream.finish();
  // printf("%zd\n", output.readableBytes());
  BOOST_CHECK_EQUAL(stream.zlibErrorCode(), Z_STREAM_END);

  muduo::net::Buffer input;
  muduo::net::ZlibInputStream inputStream(&input);
  BOOST_CHECK_EQUAL(inputStream.zlibErrorCode(), Z_OK);
  inputStream.write(&output);
  inputStream.finish();
  BOOST_CHECK_EQUAL(inputStream.zlibErrorCode(), Z_STREAM_END);
  BOOST_CHECK_EQUAL(input.readableBytes(), 50);
  BOOST_CHECK_EQUAL(output.readableBytes(), 0);
}

BOOST_AUTO_TEST_CASE(testZlibInputStream3)
{
  muduo::net::Buffer output;
  muduo::net::ZlibOutputStream stream(&output);
  BOOST_CHECK_EQUAL(stream.zlibErrorCode(), Z_OK);
  for (int i = 0; i < 1024*1024; ++i)
  {
    BOOST_CHECK(stream.write("01234567890123456789012345678901234567890123456789"));
  }
  stream.finish();
  // printf("total %zd\n", output.readableBytes());
  BOOST_CHECK_EQUAL(stream.zlibErrorCode(), Z_STREAM_END);
  muduo::net::Buffer input;
  muduo::net::ZlibInputStream inputStream(&input);
  BOOST_CHECK_EQUAL(inputStream.zlibErrorCode(), Z_OK);
  inputStream.write(output.retrieveAllAsString());
  BOOST_CHECK_EQUAL(inputStream.zlibErrorCode(), Z_STREAM_END);
  BOOST_CHECK_EQUAL(input.readableBytes(), 1024*1024*50);
}

BOOST_AUTO_TEST_CASE(testZlibInputStream4)
{
  muduo::net::Buffer output;
  muduo::net::ZlibOutputStream stream(&output);
  BOOST_CHECK_EQUAL(stream.zlibErrorCode(), Z_OK);
  muduo::string input;
  for (int i = 0; i < 32768; ++i)
  {
    input += "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_-"[rand() % 64];
  }

  muduo::net::Buffer inputBuffer;
  muduo::net::ZlibInputStream inputStream(&inputBuffer);

  for (int i = 0; i < 10; ++i)
  {
    BOOST_CHECK(stream.write(input));
    BOOST_CHECK(inputStream.write(output.retrieveAllAsString()));
  }
  stream.finish();
  // printf("total %zd\n", output.readableBytes());
  BOOST_CHECK_EQUAL(stream.zlibErrorCode(), Z_STREAM_END);

  inputStream.finish();
  BOOST_CHECK_EQUAL(inputStream.zlibErrorCode(), Z_STREAM_END);
}

BOOST_AUTO_TEST_CASE(testZlibInputStream5)
{
  muduo::net::Buffer output;
  muduo::net::ZlibOutputStream stream(&output);
  BOOST_CHECK_EQUAL(stream.zlibErrorCode(), Z_OK);
  muduo::string input(1024*1024, '_');
  BOOST_CHECK(stream.write(input));
  stream.finish();
  BOOST_CHECK_EQUAL(stream.zlibErrorCode(), Z_STREAM_END);

  muduo::net::Buffer inputBuffer;
  muduo::net::ZlibInputStream inputStream(&inputBuffer);

  for (int i = 0; i < 64; ++i)
  {
    BOOST_CHECK(inputStream.write(output.retrieveAllAsString()));
  }
  printf("bufsiz %d\n", inputStream.internalOutputBufferSize());
  LOG_INFO << "total_in " << inputStream.inputBytes();
  LOG_INFO << "total_out " << inputStream.outputBytes();
  inputStream.finish();
  printf("total %zd\n", inputBuffer.readableBytes());
  BOOST_CHECK_EQUAL(inputStream.zlibErrorCode(), Z_STREAM_END);
}
