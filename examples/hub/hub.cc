#include "codec.h"

#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>

#include <boost/bind.hpp>

#include <map>
#include <set>
#include <stdio.h>

using namespace muduo;
using namespace muduo::net;

namespace
{
enum ParseResult
{
  kError,
  kSuccess,
  kContinue,
};

ParseResult parseMessage(Buffer* buf, string* cmd, string* topic, string* content)
{
  ParseResult result = kError;
  const char* crlf = buf->findCRLF();
  if (crlf)
  {
    const char* space = std::find(buf->peek(), crlf, ' ');
    if (space != crlf)
    {
      cmd->assign(buf->peek(), space);
      topic->assign(space+1, crlf);
      if (*cmd == "pub")
      {
        const char* start = crlf + 2;
        crlf = buf->findCRLF(start);
        if (crlf)
        {
          content->assign(start, crlf);
          buf->retrieveUntil(crlf+2);
          result = kSuccess;
        }
        else
        {
          result = kContinue;
        }
      }
      else
      {
        buf->retrieveUntil(crlf+2);
        result = kSuccess;
      }
    }
    else
    {
      result = kError;
    }
  }
  else
  {
    result = kContinue;
  }
  return result;
}
}

namespace pubsub
{

typedef std::set<string> ConnectionSubscription;

class Topic : public muduo::copyable
{
 public:
  Topic(const string topic)
    : topic_(topic)
  {
  }

  void add(const TcpConnectionPtr& conn)
  {
    audiences_.insert(conn);
    if (lastPubTime_.valid())
    {
      conn->send(makeMessage());
    }
  }

  void remove(const TcpConnectionPtr& conn)
  {
    audiences_.erase(conn);
  }

  void publish(const string& content, Timestamp time)
  {
    content_ = content;
    lastPubTime_ = time;
    string message = makeMessage();
    for (std::set<TcpConnectionPtr>::iterator it = audiences_.begin();
         it != audiences_.end();
         ++it)
    {
      (*it)->send(message);
    }
  }

 private:

  string makeMessage()
  {
    return "pub " + topic_ + "\r\n" + content_ + "\r\n";
  }

  string topic_;
  string content_;
  Timestamp lastPubTime_;
  std::set<TcpConnectionPtr> audiences_;
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

  void start()
  {
    server_.start();
  }

 private:
  void onConnection(const TcpConnectionPtr& conn)
  {
    if (conn->connected())
    {
      conn->setContext(ConnectionSubscription());
    }
    else
    {
      ConnectionSubscription& connSub
        = boost::any_cast<ConnectionSubscription&>(conn->getContext());
      for (ConnectionSubscription::iterator it = connSub.begin();
           it != connSub.end();
           ++it)
      {
        doUnsubscribe(conn, *it);
      }
    }
  }

  void onMessage(const TcpConnectionPtr& conn,
                 Buffer* buf,
                 Timestamp receiveTime)
  {
    string cmd;
    string topic;
    string content;
    ParseResult result = parseMessage(buf, &cmd, &topic, &content);
    if (result == kSuccess)
    {
      if (cmd == "pub")
      {
        doPublish(conn, topic, content, receiveTime);
      }
      else if (cmd == "sub")
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
    else if (result == kError)
    {
      conn->shutdown();
    }
  }

  void timePublish()
  {
  }

  void doSubscribe(const TcpConnectionPtr& conn,
                   const string& topic)
  {
    ConnectionSubscription& connSub
      = boost::any_cast<ConnectionSubscription&>(conn->getContext());

    connSub.insert(topic);
    getTopic(topic).add(conn);
  }

  void doUnsubscribe(const TcpConnectionPtr& conn,
                     const string& topic)
  {
    ConnectionSubscription& connSub
      = boost::any_cast<ConnectionSubscription&>(conn->getContext());
    connSub.erase(topic);
    getTopic(topic).remove(conn);
  }

  void doPublish(const TcpConnectionPtr& conn,
                 const string& topic,
                 const string& content,
                 Timestamp time)
  {
    getTopic(topic).publish(content, time);
  }

  Topic& getTopic(const string& topic)
  {
    std::map<string, Topic>::iterator it = topics_.find(topic);
    if (it == topics_.end())
    {
      it = topics_.insert(make_pair(topic, Topic(topic))).first;
    }
    return it->second;
  }

  EventLoop* loop_;
  TcpServer server_;
  std::map<string, Topic> topics_;
};

}

int main(int argc, char* argv[])
{
  if (argc > 1)
  {
    int port = atoi(argv[1]);
    EventLoop loop;
    if (argc > 2)
    {
      int inspectPort = atoi(argv[2]);
    }
    pubsub::PubSubServer server(&loop, InetAddress(port));
    server.start();
    loop.loop();
  }
  else
  {
    printf("Usage: %s pubsub_port [inspect_port]\n", argv[0]);
  }
}

