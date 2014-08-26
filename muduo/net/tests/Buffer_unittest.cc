#include <muduo/net/Buffer.h>

//#define BOOST_TEST_MODULE BufferTest
#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

using muduo::string;
using muduo::net::Buffer;

BOOST_AUTO_TEST_CASE(testBufferAppendRetrieve)
{
  Buffer buf;
  BOOST_CHECK_EQUAL(buf.readableBytes(), 0);
  BOOST_CHECK_EQUAL(buf.writableBytes(), Buffer::kInitialSize);
  BOOST_CHECK_EQUAL(buf.prependableBytes(), Buffer::kCheapPrepend);

  const string str(200, 'x');
  buf.append(str);
  BOOST_CHECK_EQUAL(buf.readableBytes(), str.size());
  BOOST_CHECK_EQUAL(buf.writableBytes(), Buffer::kInitialSize - str.size());
  BOOST_CHECK_EQUAL(buf.prependableBytes(), Buffer::kCheapPrepend);

  const string str2 =  buf.retrieveAsString(50);
  BOOST_CHECK_EQUAL(str2.size(), 50);
  BOOST_CHECK_EQUAL(buf.readableBytes(), str.size() - str2.size());
  BOOST_CHECK_EQUAL(buf.writableBytes(), Buffer::kInitialSize - str.size());
  BOOST_CHECK_EQUAL(buf.prependableBytes(), Buffer::kCheapPrepend + str2.size());

  buf.append(str);
  BOOST_CHECK_EQUAL(buf.readableBytes(), 2*str.size() - str2.size());
  BOOST_CHECK_EQUAL(buf.writableBytes(), Buffer::kInitialSize - 2*str.size());
  BOOST_CHECK_EQUAL(buf.prependableBytes(), Buffer::kCheapPrepend + str2.size());

  const string str3 =  buf.retrieveAllAsString();
  BOOST_CHECK_EQUAL(str3.size(), 350);
  BOOST_CHECK_EQUAL(buf.readableBytes(), 0);
  BOOST_CHECK_EQUAL(buf.writableBytes(), Buffer::kInitialSize);
  BOOST_CHECK_EQUAL(buf.prependableBytes(), Buffer::kCheapPrepend);
}

BOOST_AUTO_TEST_CASE(testBufferGrow)
{
  Buffer buf;
  buf.append(string(400, 'y'));
  BOOST_CHECK_EQUAL(buf.readableBytes(), 400);
  BOOST_CHECK_EQUAL(buf.writableBytes(), Buffer::kInitialSize-400);

  buf.retrieve(50);
  BOOST_CHECK_EQUAL(buf.readableBytes(), 350);
  BOOST_CHECK_EQUAL(buf.writableBytes(), Buffer::kInitialSize-400);
  BOOST_CHECK_EQUAL(buf.prependableBytes(), Buffer::kCheapPrepend+50);

  buf.append(string(1000, 'z'));
  BOOST_CHECK_EQUAL(buf.readableBytes(), 1350);
  BOOST_CHECK_EQUAL(buf.writableBytes(), 0);
  BOOST_CHECK_EQUAL(buf.prependableBytes(), Buffer::kCheapPrepend+50); // FIXME

  buf.retrieveAll();
  BOOST_CHECK_EQUAL(buf.readableBytes(), 0);
  BOOST_CHECK_EQUAL(buf.writableBytes(), 1400); // FIXME
  BOOST_CHECK_EQUAL(buf.prependableBytes(), Buffer::kCheapPrepend);
}

BOOST_AUTO_TEST_CASE(testBufferInsideGrow)
{
  Buffer buf;
  buf.append(string(800, 'y'));
  BOOST_CHECK_EQUAL(buf.readableBytes(), 800);
  BOOST_CHECK_EQUAL(buf.writableBytes(), Buffer::kInitialSize-800);

  buf.retrieve(500);
  BOOST_CHECK_EQUAL(buf.readableBytes(), 300);
  BOOST_CHECK_EQUAL(buf.writableBytes(), Buffer::kInitialSize-800);
  BOOST_CHECK_EQUAL(buf.prependableBytes(), Buffer::kCheapPrepend+500);

  buf.append(string(300, 'z'));
  BOOST_CHECK_EQUAL(buf.readableBytes(), 600);
  BOOST_CHECK_EQUAL(buf.writableBytes(), Buffer::kInitialSize-600);
  BOOST_CHECK_EQUAL(buf.prependableBytes(), Buffer::kCheapPrepend);
}

BOOST_AUTO_TEST_CASE(testBufferShrink)
{
  Buffer buf;
  buf.append(string(2000, 'y'));
  BOOST_CHECK_EQUAL(buf.readableBytes(), 2000);
  BOOST_CHECK_EQUAL(buf.writableBytes(), 0);
  BOOST_CHECK_EQUAL(buf.prependableBytes(), Buffer::kCheapPrepend);

  buf.retrieve(1500);
  BOOST_CHECK_EQUAL(buf.readableBytes(), 500);
  BOOST_CHECK_EQUAL(buf.writableBytes(), 0);
  BOOST_CHECK_EQUAL(buf.prependableBytes(), Buffer::kCheapPrepend+1500);

  buf.shrink(0);
  BOOST_CHECK_EQUAL(buf.readableBytes(), 500);
  BOOST_CHECK_EQUAL(buf.writableBytes(), Buffer::kInitialSize-500);
  BOOST_CHECK_EQUAL(buf.retrieveAllAsString(), string(500, 'y'));
  BOOST_CHECK_EQUAL(buf.prependableBytes(), Buffer::kCheapPrepend);
}

BOOST_AUTO_TEST_CASE(testBufferPrepend)
{
  Buffer buf;
  buf.append(string(200, 'y'));
  BOOST_CHECK_EQUAL(buf.readableBytes(), 200);
  BOOST_CHECK_EQUAL(buf.writableBytes(), Buffer::kInitialSize-200);
  BOOST_CHECK_EQUAL(buf.prependableBytes(), Buffer::kCheapPrepend);

  int x = 0;
  buf.prepend(&x, sizeof x);
  BOOST_CHECK_EQUAL(buf.readableBytes(), 204);
  BOOST_CHECK_EQUAL(buf.writableBytes(), Buffer::kInitialSize-200);
  BOOST_CHECK_EQUAL(buf.prependableBytes(), Buffer::kCheapPrepend - 4);
}

BOOST_AUTO_TEST_CASE(testBufferReadInt)
{
  Buffer buf;
  buf.append("HTTP");

  BOOST_CHECK_EQUAL(buf.readableBytes(), 4);
  BOOST_CHECK_EQUAL(buf.peekInt8(), 'H');
  int top16 = buf.peekInt16();
  BOOST_CHECK_EQUAL(top16, 'H'*256 + 'T');
  BOOST_CHECK_EQUAL(buf.peekInt32(), top16*65536 + 'T'*256 + 'P');

  BOOST_CHECK_EQUAL(buf.readInt8(), 'H');
  BOOST_CHECK_EQUAL(buf.readInt16(), 'T'*256 + 'T');
  BOOST_CHECK_EQUAL(buf.readInt8(), 'P');
  BOOST_CHECK_EQUAL(buf.readableBytes(), 0);
  BOOST_CHECK_EQUAL(buf.writableBytes(), Buffer::kInitialSize);

  buf.appendInt8(-1);
  buf.appendInt16(-1);
  buf.appendInt32(-1);
  BOOST_CHECK_EQUAL(buf.readableBytes(), 7);
  BOOST_CHECK_EQUAL(buf.readInt8(), -1);
  BOOST_CHECK_EQUAL(buf.readInt32(), -1);
  BOOST_CHECK_EQUAL(buf.readInt16(), -1);
}

BOOST_AUTO_TEST_CASE(testBufferFindEOL)
{
  Buffer buf;
  buf.append(string(100000, 'x'));
  const char* null = NULL;
  BOOST_CHECK_EQUAL(buf.findEOL(), null);
  BOOST_CHECK_EQUAL(buf.findEOL(buf.peek()+90000), null);
}

#ifdef __GXX_EXPERIMENTAL_CXX0X__
void output(Buffer&& buf, const void* inner)
{
  Buffer newbuf(std::move(buf));
  // printf("New Buffer at %p, inner %p\n", &newbuf, newbuf.peek());
  BOOST_CHECK_EQUAL(inner, newbuf.peek());
}

// NOTE: This test fails in g++ 4.4, passes in g++ 4.6.
BOOST_AUTO_TEST_CASE(testMove)
{
  Buffer buf;
  buf.append("muduo", 5);
  const void* inner = buf.peek();
  // printf("Buffer at %p, inner %p\n", &buf, inner);
  output(std::move(buf), inner);
}
#endif
