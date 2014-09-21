#include "Hiredis.h"

#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>

using namespace hiredis;
using namespace muduo;
using namespace muduo::net;

void connectCallback(const redisAsyncContext *c, int status)
{
  if (status != REDIS_OK) {
    LOG_ERROR << "Error:" << c->errstr;
  }

  LOG_DEBUG << "Connected...";
}

void disconnectCallback(const redisAsyncContext *c, int status)
{
  if (status != REDIS_OK) {
    LOG_ERROR << "Error:" << c->errstr;
  }

  LOG_DEBUG << "Disconnected...";
}

int main(int argc, char **argv)
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

  loop.loop();

  return 0;
}
