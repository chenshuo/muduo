#include <muduo/net/http/HttpContext.h>
#include <muduo/net/Buffer.h>

//#define BOOST_TEST_MODULE BufferTest
#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK

#include <gtest/gtest.h>

using muduo::string;
using muduo::Timestamp;
using muduo::net::Buffer;
using muduo::net::HttpContext;
using muduo::net::HttpRequest;

TEST(testParseRequestAllInOne, parseRequest)
{
  HttpContext context;
  Buffer input;
  input.append("GET /index.html HTTP/1.1\r\n"
       "Host: www.chenshuo.com\r\n"
       "\r\n");

  GTEST_CHECK_(context.parseRequest(&input, Timestamp::now()));
  GTEST_CHECK_(context.gotAll());
  const HttpRequest& request = context.request();
  EXPECT_EQ(request.method(), HttpRequest::kGet);
  EXPECT_EQ(request.path(), string("/index.html"));
  EXPECT_EQ(request.getVersion(), HttpRequest::kHttp11);
  EXPECT_EQ(request.getHeader("Host"), string("www.chenshuo.com"));
  EXPECT_EQ(request.getHeader("User-Agent"), string(""));
}

TEST(testParseRequestInTwoPieces, parseRequest)
{
  string all("GET /index.html HTTP/1.1\r\n"
       "Host: www.chenshuo.com\r\n"
       "\r\n");

  for (size_t sz1 = 0; sz1 < all.size(); ++sz1)
  {
    HttpContext context;
    Buffer input;
    input.append(all.c_str(), sz1);
    GTEST_CHECK_(context.parseRequest(&input, Timestamp::now()));
    GTEST_CHECK_(!context.gotAll());

    size_t sz2 = all.size() - sz1;
    input.append(all.c_str() + sz1, sz2);
    GTEST_CHECK_(context.parseRequest(&input, Timestamp::now()));
    GTEST_CHECK_(context.gotAll());
    const HttpRequest& request = context.request();
    EXPECT_EQ(request.method(), HttpRequest::kGet);
    EXPECT_EQ(request.path(), string("/index.html"));
    EXPECT_EQ(request.getVersion(), HttpRequest::kHttp11);
    EXPECT_EQ(request.getHeader("Host"), string("www.chenshuo.com"));
    EXPECT_EQ(request.getHeader("User-Agent"), string(""));
  }
}

TEST(testParseRequestEmptyHeaderValue, parseRequest)
{
  HttpContext context;
  Buffer input;
  input.append("GET /index.html HTTP/1.1\r\n"
       "Host: www.chenshuo.com\r\n"
       "User-Agent:\r\n"
       "Accept-Encoding: \r\n"
       "\r\n");

  GTEST_CHECK_(context.parseRequest(&input, Timestamp::now()));
  GTEST_CHECK_(context.gotAll());
  const HttpRequest& request = context.request();
  EXPECT_EQ(request.method(), HttpRequest::kGet);
  EXPECT_EQ(request.path(), string("/index.html"));
  EXPECT_EQ(request.getVersion(), HttpRequest::kHttp11);
  EXPECT_EQ(request.getHeader("Host"), string("www.chenshuo.com"));
  EXPECT_EQ(request.getHeader("User-Agent"), string(""));
  EXPECT_EQ(request.getHeader("Accept-Encoding"), string(""));
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
