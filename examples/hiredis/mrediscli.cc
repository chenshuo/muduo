#include "Hiredis.h"

#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>

#include <string>

using namespace hiredis;
using namespace muduo;
using namespace muduo::net;

// muduo/base/LogStream.cc
// Efficient Integer to String Conversions, by Matthew Wilson.
template<typename T>
size_t convert(char buf[], T value)
{
  const char digits[] = "9876543210123456789";
  const char* zero = digits + 9;
  T i = value;
  char* p = buf;

  do
  {
    int lsd = static_cast<int>(i % 10);
    i /= 10;
    *p++ = zero[lsd];
  } while (i != 0);

  if (value < 0)
  {
    *p++ = '-';
  }
  *p = '\0';
  std::reverse(buf, p);

  return p - buf;
}

template<typename T>
string toString(T value)
{
  char buf[32];
  convert(buf, value);
  return buf;
}

string redisReplyToString(const redisReply* reply)
{
  const char* types[] = {"", "REDIS_REPLY_STRING", "REDIS_REPLY_ARRAY",
                         "REDIS_REPLY_INTEGER", "REDIS_REPLY_NIL",
                         "REDIS_REPLY_STATUS", "REDIS_REPLY_ERROR"
                        };
  string str = "";
  if (!reply) return str;

  str += types[reply->type] + string("(") + toString(reply->type) + ") ";

  str += "{ ";
  if (reply->type == REDIS_REPLY_STRING ||
      reply->type == REDIS_REPLY_STATUS ||
      reply->type == REDIS_REPLY_ERROR)
  {
    str += string(reply->str, reply->len);
  }
  else if (reply->type == REDIS_REPLY_INTEGER)
  {
    str += toString(reply->type);
  }
  else if (reply->type == REDIS_REPLY_ARRAY)
  {
    str += toString(reply->elements) + " ";
    for (size_t i = 0; i < reply->elements; i++)
    {
      str += " " + redisReplyToString(reply->element[i]);
    }
  }
  str += " }";

  return str;
}

void connectCallback(const redisAsyncContext* c, int status)
{
  if (status != REDIS_OK) {
    LOG_ERROR << "Error:" << c->errstr;
  }

  LOG_DEBUG << "Connected...";
}

void disconnectCallback(const redisAsyncContext* c, int status)
{
  if (status != REDIS_OK) {
    LOG_ERROR << "Error:" << c->errstr;
  }

  LOG_DEBUG << "Disconnected...";
}

void timeCallback(redisAsyncContext* c, void* r, void* privdata)
{
  redisReply* reply = static_cast<redisReply*>(r);
  //LOG_TRACE << "time " << reply->element[0]->str << "." << reply->element[1]->str;
  LOG_TRACE << "time " << redisReplyToString(reply);
}

void echoCallback(redisAsyncContext* c, void* r, void* privdata)
{
  redisReply* reply = static_cast<redisReply*>(r);
  string* echo = static_cast<string*>(privdata);
  LOG_TRACE << *echo << " " << redisReplyToString(reply);
}

void dbsizeCallback(redisAsyncContext* c, void* r, void* privdata)
{
  redisReply* reply = static_cast<redisReply*>(r);
  LOG_TRACE << "dbsize " << redisReplyToString(reply);
}

void selectCallback(redisAsyncContext* c, void* r, void* privdata)
{
  redisReply* reply = static_cast<redisReply*>(r);
  uint16_t* index = static_cast<uint16_t*>(privdata);
  LOG_TRACE << "select " << *index << " " << redisReplyToString(reply);
}

void authCallback(redisAsyncContext* c, void* r, void* privdata)
{
  redisReply* reply = static_cast<redisReply*>(r);
  string* password = static_cast<string*>(privdata);
  LOG_TRACE << "auth " << *password << " " << redisReplyToString(reply);
}

int main(int argc, char** argv)
{
  //Logger::setLogLevel(Logger::DEBUG);
  Logger::setLogLevel(Logger::TRACE);

  EventLoop loop;

  hiredis::Hiredis hiredis(&loop, "127.0.0.1", 6379);
  //hiredis::Hiredis hiredis(&loop, "127.0.0.1", 6378);

  hiredis.setConnectCallback(connectCallback);
  hiredis.setDisconnectCallback(disconnectCallback);
  //hiredis.ping();
  loop.runEvery(1.0, boost::bind(&Hiredis::ping, &hiredis));

  redisAsyncCommand(hiredis.context(), timeCallback, NULL, "time");

  string hi = "hi";
  redisAsyncCommand(hiredis.context(), echoCallback, &hi, "echo %s", hi.data());

  redisAsyncCommand(hiredis.context(), dbsizeCallback, NULL, "dbsize");

  uint16_t index = 8;
  redisAsyncCommand(hiredis.context(), selectCallback, &index, "select %d", index);

  string password = "password";
  redisAsyncCommand(hiredis.context(), authCallback, &password, "auth %s", password.data());

  loop.loop();

  return 0;
}
