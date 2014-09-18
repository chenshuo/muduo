#ifndef MUDUO_EXAMPLES_REDIS_ADAPTER_MUDUO_H
#define MUDUO_EXAMPLES_REDIS_ADAPTER_MUDUO_H

#include <muduo/base/Timestamp.h>
#include <muduo/net/Channel.h>
#include <muduo/net/EventLoop.h>

#include <boost/bind.hpp>

#include <hiredis/hiredis.h>
#include <hiredis/async.h>

using muduo::Timestamp;
using muduo::net::Channel;
using muduo::net::EventLoop;

struct redisMuduoEvents {
  redisAsyncContext *context;
  Channel *channel;

  void redisMuduoReadEvent(Timestamp receiveTime)
  {
    redisAsyncHandleRead(context);
  }

  void redisMuduoWriteEvent()
  {
    redisAsyncHandleWrite(context);
  }

};

static void redisMuduoAddRead(void *privdata)
{
  redisMuduoEvents *e = static_cast<redisMuduoEvents*>(privdata);
  e->channel->enableReading();
}

static void redisMuduoDelRead(void *privdata)
{
  redisMuduoEvents *e = static_cast<redisMuduoEvents*>(privdata);
  e->channel->disableReading();
}

static void redisMuduoAddWrite(void *privdata)
{
  redisMuduoEvents *e = static_cast<redisMuduoEvents*>(privdata);
  e->channel->enableWriting();
}

static void redisMuduoDelWrite(void *privdata)
{
  redisMuduoEvents *e = static_cast<redisMuduoEvents*>(privdata);
  e->channel->disableWriting();
}

static void redisMuduoCleanup(void *privdata)
{
  redisMuduoEvents *e = static_cast<redisMuduoEvents*>(privdata);
  e->channel->disableAll();

  delete e;
}

static int redisMuduoAttach(redisAsyncContext *ac, EventLoop *loop)
{
  redisContext *c = &(ac->c);

  /* Nothing should be attached when something is already attached */
  if (ac->ev.data != NULL)
    return REDIS_ERR;

  /* Create container for context and r/w events */
  redisMuduoEvents *e = new redisMuduoEvents;
  e->context = ac;

  /* Register functions to start/stop listening for events */
  ac->ev.addRead = redisMuduoAddRead;
  ac->ev.delRead = redisMuduoDelRead;
  ac->ev.addWrite = redisMuduoAddWrite;
  ac->ev.delWrite = redisMuduoDelWrite;
  ac->ev.cleanup = redisMuduoCleanup;
  ac->ev.data = e;

  /* Initialize and install read/write events */
  Channel *channel = new Channel(loop, c->fd);
  e->channel = channel;

  channel->enableReading();
  channel->enableWriting();

  channel->setReadCallback(
      boost::bind(&redisMuduoEvents::redisMuduoReadEvent, e, _1));
  channel->setWriteCallback(
      boost::bind(&redisMuduoEvents::redisMuduoWriteEvent, e));

  return REDIS_OK;
}

#endif  // MUDUO_EXAMPLES_REDIS_ADAPTER_MUDUO_H
