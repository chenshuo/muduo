#include <muduo/net/ZlibStream.h>

#include <muduo/base/Logging.h>
#include <gtest/gtest.h>

#include <stdio.h>

TEST(testZlibOutputStream, test)
{
  muduo::net::Buffer output;
  {
    muduo::net::ZlibOutputStream stream(&output);
    EXPECT_EQ(output.readableBytes(), (unsigned long)0);
  }
  EXPECT_EQ(output.readableBytes(), (unsigned long)8);
}

TEST(testZlibOutputStream1, test1)
{
  muduo::net::Buffer output;
  muduo::net::ZlibOutputStream stream(&output);
  EXPECT_EQ(stream.zlibErrorCode(), Z_OK);
  stream.finish();
  EXPECT_EQ(stream.zlibErrorCode(), Z_STREAM_END);
}

TEST(testZlibOutputStream2, test2)
{
  muduo::net::Buffer output;
  muduo::net::ZlibOutputStream stream(&output);
  EXPECT_EQ(stream.zlibErrorCode(), Z_OK);
  GTEST_CHECK_(stream.write("01234567890123456789012345678901234567890123456789"));
  stream.finish();
  // printf("%zd\n", output.readableBytes());
  EXPECT_EQ(stream.zlibErrorCode(), Z_STREAM_END);
}

TEST(testZlibOutputStream3, test3)
{
  muduo::net::Buffer output;
  muduo::net::ZlibOutputStream stream(&output);
  EXPECT_EQ(stream.zlibErrorCode(), Z_OK);
  for (int i = 0; i < 1024*1024; ++i)
  {
    GTEST_CHECK_(stream.write("01234567890123456789012345678901234567890123456789"));
  }
  stream.finish();
  // printf("total %zd\n", output.readableBytes());
  EXPECT_EQ(stream.zlibErrorCode(), Z_STREAM_END);
}

TEST(testZlibOutputStream4, test4)
{
  muduo::net::Buffer output;
  muduo::net::ZlibOutputStream stream(&output);
  EXPECT_EQ(stream.zlibErrorCode(), Z_OK);
  muduo::string input;
  for (int i = 0; i < 32768; ++i)
  {
    input += "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_-"[rand() % 64];
  }

  for (int i = 0; i < 10; ++i)
  {
    GTEST_CHECK_(stream.write(input));
  }
  stream.finish();
  // printf("total %zd\n", output.readableBytes());
  EXPECT_EQ(stream.zlibErrorCode(), Z_STREAM_END);
}

TEST(testZlibOutputStream5, test5)
{
  muduo::net::Buffer output;
  muduo::net::ZlibOutputStream stream(&output);
  EXPECT_EQ(stream.zlibErrorCode(), Z_OK);
  muduo::string input(1024*1024, '_');
  for (int i = 0; i < 64; ++i)
  {
    GTEST_CHECK_(stream.write(input));
  }
  printf("bufsiz %d\n", stream.internalOutputBufferSize());
  LOG_INFO << "total_in " << stream.inputBytes();
  LOG_INFO << "total_out " << stream.outputBytes();
  stream.finish();
  printf("total %zd\n", output.readableBytes());
  EXPECT_EQ(stream.zlibErrorCode(), Z_STREAM_END);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

