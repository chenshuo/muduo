#include "Hiredis.h"
#include <muduo/base/Logging.h>
#include <muduo/net/Channel.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/SocketsOps.h>

using namespace hiredis;
using namespace muduo;
using namespace muduo::net;

static void dummy(const boost::shared_ptr<Channel>&)
{
}

Hiredis::Hiredis(EventLoop* loop, const string& ip, uint16_t port)
  : loop_(loop),
    serverAddr_(ip, port),
    context_(NULL)
{
  connect();
}

Hiredis::~Hiredis()
{
  assert(!channel_ || channel_->isNoneEvent());
}

void Hiredis::connect()
{
  assert(!context_);

  context_ = redisAsyncConnect(serverAddr_.toIp().c_str(), serverAddr_.toPort());

  context_->ev.addRead = addRead;
  context_->ev.delRead = delRead;
  context_->ev.addWrite = addWrite;
  context_->ev.delWrite = delWrite;
  context_->ev.cleanup = cleanup;
  context_->ev.data = this;

  setChannel();

  assert(context_->onConnect == NULL);
  assert(context_->onDisconnect == NULL);
  ::redisAsyncSetConnectCallback(context_, connectCallback);
  ::redisAsyncSetDisconnectCallback(context_, disconnectCallback);
}

void Hiredis::setChannel()
{
  assert(!channel_);
  channel_.reset(new Channel(loop_, context_->c.fd));
  channel_->setReadCallback(boost::bind(&Hiredis::handleRead, this, _1));
  channel_->setWriteCallback(boost::bind(&Hiredis::handleWrite, this));
}

void Hiredis::removeChannel()
{
  channel_->disableAll();
  channel_->remove();
  loop_->queueInLoop(boost::bind(dummy, channel_));
  channel_.reset();
}

void Hiredis::handleRead(muduo::Timestamp receiveTime)
{
  LOG_TRACE << "receiveTime = " << receiveTime.toString();
  ::redisAsyncHandleRead(context_);
}

void Hiredis::handleWrite()
{
  if (!(context_->c.flags & REDIS_CONNECTED))
  {
    removeChannel();
  }
  ::redisAsyncHandleWrite(context_);
}


void Hiredis::connectCallback(const redisAsyncContext* ac, int status)
{
  Hiredis* hiredis = static_cast<Hiredis*>(ac->ev.data);

  if (status != REDIS_OK)
  {
    LOG_ERROR << ac->errstr << " failed to connect to " << hiredis->serverAddress().toIpPort();
  }
  else
  {
    InetAddress localAddr = sockets::getLocalAddr(ac->c.fd);
    InetAddress peerAddr = sockets::getPeerAddr(ac->c.fd);

    LOG_TRACE << localAddr.toIpPort() << " -> "
              << peerAddr.toIpPort() << " is UP";
    hiredis->setChannel();
  }

  if (hiredis->connectCallback())
  {
    hiredis->connectCallback()(ac, status);
  }
}

void Hiredis::disconnectCallback(const redisAsyncContext* ac, int status)
{
  Hiredis* hiredis = static_cast<Hiredis*>(ac->ev.data);

  InetAddress localAddr = sockets::getLocalAddr(ac->c.fd);
  InetAddress peerAddr = sockets::getPeerAddr(ac->c.fd);

  LOG_TRACE << localAddr.toIpPort() << " -> "
            << peerAddr.toIpPort() << " is DOWN";
  hiredis->removeChannel();

  if (hiredis->disconnectCallback())
  {
    hiredis->disconnectCallback()(ac, status);
  }
}

void Hiredis::addRead(void* privdata)
{
  Hiredis* hiredis = static_cast<Hiredis*>(privdata);
  hiredis->getChannel()->enableReading();
}

void Hiredis::delRead(void* privdata)
{
  Hiredis* hiredis = static_cast<Hiredis*>(privdata);
  hiredis->getChannel()->disableReading();
}

void Hiredis::addWrite(void* privdata)
{
  Hiredis* hiredis = static_cast<Hiredis*>(privdata);
  hiredis->getChannel()->enableWriting();
}

void Hiredis::delWrite(void* privdata)
{
  Hiredis* hiredis = static_cast<Hiredis*>(privdata);
  hiredis->getChannel()->disableWriting();
}

void Hiredis::cleanup(void* privdata)
{
  //Hiredis* hiredis = static_cast<Hiredis*>(privdata);
  //hiredis->removeChannel();
}

// command
int Hiredis::command(CommandCallback cb, const muduo::StringPiece& cmd)
{
  commandCb_ = cb;
  return redisAsyncCommand(context_, commandCallback, NULL, cmd.data());
}

void Hiredis::commandCallback(redisAsyncContext* ac, void* r, void* privdata)
{
  Hiredis* hiredis = static_cast<Hiredis*>(ac->ev.data);
  redisReply* reply = static_cast<redisReply*>(r);
  hiredis->commandCallback()(ac, reply, privdata);
}

int Hiredis::ping()
{
  return command(boost::bind(&Hiredis::pingCallback, this, _1, _2, _3), "PING");
}

void Hiredis::pingCallback(redisAsyncContext* ac, redisReply* reply, void* privdata)
{
  LOG_TRACE << reply->str;
}
