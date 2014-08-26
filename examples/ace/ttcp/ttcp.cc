#include <examples/ace/ttcp/common.h>

#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpClient.h>
#include <muduo/net/TcpServer.h>

#include <boost/bind.hpp>

#include <stdio.h>

using namespace muduo;
using namespace muduo::net;

EventLoop* g_loop;

struct Context
{
  int count;
  int64_t bytes;
  SessionMessage session;
  Buffer output;

  Context()
    : count(0),
      bytes(0)
  {
    session.number = 0;
    session.length = 0;
  }
};

/////////////////////////////////////////////////////////////////////
// T R A N S M I T
/////////////////////////////////////////////////////////////////////

namespace trans
{

void onConnection(const Options& opt, const TcpConnectionPtr& conn)
{
  if (conn->connected())
  {
    printf("connected\n");
    Context context;
    context.count = 1;
    context.bytes = opt.length;
    context.session.number = opt.number;
    context.session.length = opt.length;
    context.output.appendInt32(opt.length);
    context.output.ensureWritableBytes(opt.length);
    for (int i = 0; i < opt.length; ++i)
    {
      context.output.beginWrite()[i] = "0123456789ABCDEF"[i % 16];
    }
    context.output.hasWritten(opt.length);
    conn->setContext(context);

    SessionMessage sessionMessage = { 0, 0 };
    sessionMessage.number = htonl(opt.number);
    sessionMessage.length = htonl(opt.length);
    conn->send(&sessionMessage, sizeof(sessionMessage));

    conn->send(context.output.toStringPiece());
  }
  else
  {
    const Context& context = boost::any_cast<Context>(conn->getContext());
    LOG_INFO << "payload bytes " << context.bytes;
    conn->getLoop()->quit();
  }
}

void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time)
{
  Context* context = boost::any_cast<Context>(conn->getMutableContext());
  while (buf->readableBytes() >= sizeof(int32_t))
  {
    int32_t length = buf->readInt32();
    if (length == context->session.length)
    {
      if (context->count < context->session.number)
      {
        conn->send(context->output.toStringPiece());
        ++context->count;
        context->bytes += length;
      }
      else
      {
        conn->shutdown();
        break;
      }
    }
    else
    {
      conn->shutdown();
      break;
    }
  }
}

}

void transmit(const Options& opt)
{
  InetAddress addr(opt.port);
  if (!InetAddress::resolve(opt.host, &addr))
  {
    LOG_FATAL << "Unable to resolve " << opt.host;
  }
  muduo::Timestamp start(muduo::Timestamp::now());
  EventLoop loop;
  g_loop = &loop;
  TcpClient client(&loop, addr, "TtcpClient");
  client.setConnectionCallback(
      boost::bind(&trans::onConnection, opt, _1));
  client.setMessageCallback(
      boost::bind(&trans::onMessage, _1, _2, _3));
  client.connect();
  loop.loop();
  double elapsed = timeDifference(muduo::Timestamp::now(), start);
  double total_mb = 1.0 * opt.length * opt.number / 1024 / 1024;
  printf("%.3f MiB transferred\n%.3f MiB/s\n", total_mb, total_mb / elapsed);
}

/////////////////////////////////////////////////////////////////////
// R E C E I V E
/////////////////////////////////////////////////////////////////////

namespace receiving
{

void onConnection(const TcpConnectionPtr& conn)
{
  if (conn->connected())
  {
    Context context;
    conn->setContext(context);
  }
  else
  {
    const Context& context = boost::any_cast<Context>(conn->getContext());
    LOG_INFO << "payload bytes " << context.bytes;
    conn->getLoop()->quit();
  }
}

void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time)
{
  while (buf->readableBytes() >= sizeof(int32_t))
  {
    Context* context = boost::any_cast<Context>(conn->getMutableContext());
    SessionMessage& session = context->session;
    if (session.number == 0 && session.length == 0)
    {
      if (buf->readableBytes() >= sizeof(SessionMessage))
      {
        session.number = buf->readInt32();
        session.length = buf->readInt32();
        context->output.appendInt32(session.length);
        printf("receive number = %d\nreceive length = %d\n",
               session.number, session.length);
      }
      else
      {
        break;
      }
    }
    else
    {
      const unsigned total_len = session.length + static_cast<int>(sizeof(int32_t));
      const int32_t length = buf->peekInt32();
      if (length == session.length)
      {
        if (buf->readableBytes() >= total_len)
        {
          buf->retrieve(total_len);
          conn->send(context->output.toStringPiece());
          ++context->count;
          context->bytes += length;
          if (context->count >= session.number)
          {
            conn->shutdown();
            break;
          }
        }
        else
        {
          break;
        }
      }
      else
      {
        printf("wrong length %d\n", length);
        conn->shutdown();
        break;
      }
    }
  }
}

}

void receive(const Options& opt)
{
  EventLoop loop;
  g_loop = &loop;
  InetAddress listenAddr(opt.port);
  TcpServer server(&loop, listenAddr, "TtcpReceive");
  server.setConnectionCallback(
       boost::bind(&receiving::onConnection, _1));
  server.setMessageCallback(
      boost::bind(&receiving::onMessage, _1, _2, _3));
  server.start();
  loop.loop();
}
