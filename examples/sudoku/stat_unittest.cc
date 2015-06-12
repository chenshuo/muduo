#include <muduo/base/Logging.h>
#include <muduo/base/Thread.h>
#include <muduo/base/ThreadPool.h>

#include <boost/circular_buffer.hpp>
#include <boost/noncopyable.hpp>
#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

using namespace muduo;

#include "stat.h"

#include <stdio.h>

BOOST_AUTO_TEST_CASE(testSudokuStatSameSecond)
{
  ThreadPool p;
  SudokuStat s(p);

  for (int i = 0; i < 100; ++i)
  {
    time_t start = 1234567890;
    Timestamp recv = Timestamp::fromUnixTime(start, 0);
    Timestamp send = Timestamp::fromUnixTime(start, i);
    s.recordResponse(send, recv, i % 3 != 0);
  }
  printf("same second:\n%s\n", s.report().c_str());
}

BOOST_AUTO_TEST_CASE(testSudokuStatNextSecond)
{
  ThreadPool p;
  SudokuStat s(p);

  time_t start = 1234567890;
  Timestamp recv = Timestamp::fromUnixTime(start, 0);
  Timestamp send = addTime(recv, 0.002);
  for (int i = 0; i < 10000; ++i)
  {
    s.recordResponse(send, recv, true);
    recv = addTime(send, 0.01);
    send = addTime(recv, 0.02);
  }
  printf("next second:\n%s\n", s.report().c_str());
}

BOOST_AUTO_TEST_CASE(testSudokuStatFuzz)
{
  ThreadPool p;
  SudokuStat s(p);

  time_t start = 1234567890;
  srand(static_cast<unsigned>(time(NULL)));
  for (int i = 0; i < 10000; ++i)
  {
    Timestamp recv = Timestamp::fromUnixTime(start, 0);
    Timestamp send = Timestamp::fromUnixTime(start, 200);
    s.recordResponse(send, recv, true);
    int jump = (rand() % 200) - 100;
    // printf("%4d ", jump);
    start += jump;
  }
}

BOOST_AUTO_TEST_CASE(testSudokuStatJumpAhead5)
{
  ThreadPool p;
  SudokuStat s(p);

  time_t start = 1234567890;
  Timestamp recv = Timestamp::fromUnixTime(start, 0);
  Timestamp send = Timestamp::fromUnixTime(start, 200);
  s.recordResponse(send, recv, true);

  recv = addTime(recv, 4);
  send = addTime(send, 5);
  s.recordResponse(send, recv, true);
  printf("jump ahead 5 seconds:\n%s\n", s.report().c_str());
}

BOOST_AUTO_TEST_CASE(testSudokuStatJumpAhead59)
{
  ThreadPool p;
  SudokuStat s(p);

  time_t start = 1234567890;
  Timestamp recv = Timestamp::fromUnixTime(start, 0);
  Timestamp send = Timestamp::fromUnixTime(start, 200);
  s.recordResponse(send, recv, true);

  recv = addTime(recv, 55);
  send = addTime(send, 59);
  s.recordResponse(send, recv, true);
  printf("jump ahead 59 seconds:\n%s\n", s.report().c_str());
}

BOOST_AUTO_TEST_CASE(testSudokuStatJumpAhead60)
{
  ThreadPool p;
  SudokuStat s(p);

  time_t start = 1234567890;
  Timestamp recv = Timestamp::fromUnixTime(start, 0);
  Timestamp send = Timestamp::fromUnixTime(start, 200);
  s.recordResponse(send, recv, true);

  recv = addTime(recv, 58);
  send = addTime(send, 60);
  s.recordResponse(send, recv, true);
  printf("jump ahead 60 seconds:\n%s\n", s.report().c_str());
}

BOOST_AUTO_TEST_CASE(testSudokuStatJumpBack3)
{
  ThreadPool p;
  SudokuStat s(p);

  time_t start = 1234567890;
  Timestamp recv = Timestamp::fromUnixTime(start, 0);
  Timestamp send = Timestamp::fromUnixTime(start, 200);
  s.recordResponse(send, recv, true);

  recv = addTime(recv, 9);
  send = addTime(send, 10);
  s.recordResponse(send, recv, true);

  recv = addTime(recv, -4);
  send = addTime(send, -3);
  s.recordResponse(send, recv, true);

  printf("jump back 3 seconds:\n%s\n", s.report().c_str());
}

