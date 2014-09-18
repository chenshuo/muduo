#include "adapter/muduo.h"

#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>

using namespace muduo;
using namespace muduo::net;

void getCallback(redisAsyncContext *c, void *r, void *privdata) {
  redisReply *reply = static_cast<redisReply*>(r);
  if (reply == NULL) return;
  LOG_DEBUG<<"argv["<<static_cast<char*>(privdata)<<"]: "<<reply->str;

  /* Disconnect after receiving the reply to GET */
  redisAsyncDisconnect(c);
}

void connectCallback(const redisAsyncContext *c, int status)
{
  if (status != REDIS_OK) {
    LOG_ERROR<<"Error:"<<c->errstr;
  }

  LOG_DEBUG<<"Connected...";
}

void disconnectCallback(const redisAsyncContext *c, int status)
{
  if (status != REDIS_OK) {
    LOG_ERROR<<"Error:"<<c->errstr;
  }

  LOG_DEBUG<<"Disconnected...";
}

int main(int argc, char **argv)
{
  Logger::setLogLevel(Logger::DEBUG);

  EventLoop loop;

  redisAsyncContext *c = redisAsyncConnect("127.0.0.1", 6379);

  if (c->err) {
    /* Let *c leak for now... */
    LOG_DEBUG<<"Error:"<<c->errstr;
    return 1;
  }

  redisMuduoAttach(c, &loop);

  redisAsyncSetConnectCallback(c, connectCallback);
  redisAsyncSetDisconnectCallback(c, disconnectCallback);
  redisAsyncCommand(c, NULL, NULL, "SET key %b", argv[argc-1], strlen(argv[argc-1]));
  redisAsyncCommand(c, getCallback, const_cast<char*>("end-1"), "GET key");

  loop.loop();

  return 0;
}
