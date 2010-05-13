#include "codec.h"

#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>

#include <boost/bind.hpp>

#include <stdio.h>

using namespace muduo;
using namespace muduo::net;

namespace pubsub
{
class PubSubServer;

class ClientContext : muduo::copyable
{
 public:
  ClientContext(PubSubServer* owner)
    : owner_(owner)
  {
  }

  // default copy-ctor, dtor and assignment are fine

 private:
  PubSubServer* owner_;
};

class PubSubServer : boost::noncopyable
{
 public:
  PubSubServer(muduo::net::EventLoop* loop,
               const muduo::net::InetAddress& listenAddr)
    : loop_(loop),
      server_(loop, listenAddr, "PubSubServer")
  {
    server_.setConnectionCallback(
        boost::bind(&PubSubServer::onConnection, this, _1));
    server_.setMessageCallback(
        boost::bind(&PubSubServer::onMessage, this, _1, _2, _3));
  }

  void start(int numThreads)
  {
    server_.setThreadNum(numThreads);
    server_.start();
  }

 private:
  void onConnection(const TcpConnectionPtr& conn)
  {
    if (conn->connected())
    {
      conn->setContext(ClientContext(this));
    }
    else
    {

    }
  }

  // FIXME extract paring function
  void onMessage(const TcpConnectionPtr& conn,
                 Buffer* buf,
                 Timestamp receiveTime)
  {
    const char* crlf = buf->findCRLF();
    if (crlf)
    {
      const char* space = std::find(buf->peek(), crlf, ' ');
      if (space != crlf)
      {
        string cmd(buf->peek(), space);
        string topic(space+1, crlf);
        if (cmd == "pub")
        {
          const char* start = crlf+2;
          crlf = buf->findCRLF(start);
          if (crlf)
          {
            string content(start, crlf);
            buf->retrieveUntil(crlf + 2);
            doPublish(conn, topic, content, receiveTime);
          }
          else
          {
            LOG_DEBUG << "More";
          }
        }
        else
        {
          buf->retrieveUntil(crlf + 2);
          if (cmd == "sub")
          {
            doSubscribe(conn, topic);
          }
          else if (cmd == "unsub")
          {
            doUnsubscribe(conn, topic);
          }
          else
          {
            conn->shutdown();
          }
        }
      }
      else
      {
        conn->shutdown();
      }
    }
  }

  void timePublish()
  {
  }

  void doSubscribe(const TcpConnectionPtr& conn,
                   const string& topic)
  {
    LOG_INFO << "conn " << conn->name() << " sub " << topic;
  }

  void doUnsubscribe(const TcpConnectionPtr& conn,
                     const string& topic)
  {
    LOG_INFO << "conn " << conn->name() << " unsub " << topic;
  }

  void doPublish(const TcpConnectionPtr& conn,
                 const string& topic,
                 const string& content,
                 Timestamp time)
  {
    LOG_INFO << "conn " << conn->name() << " pub " << topic;
  }

  EventLoop* loop_;
  TcpServer server_;
};

}

int main(int argc, char* argv[])
{
  if (argc > 2)
  {
    int port = atoi(argv[1]);
    int inspectPort = atoi(argv[2]);
    int threads = 0;
    if (argc > 3)
    {
      threads = atoi(argv[3]);
    }
    EventLoop loop;
    pubsub::PubSubServer server(&loop, InetAddress(port));
    server.start(threads);
    loop.loop();
  }
  else
  {
    printf("Usage: %s pubsub_port inspect_port [num_threads]\n", argv[0]);
  }
}
