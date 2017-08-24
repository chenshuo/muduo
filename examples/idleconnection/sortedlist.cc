#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>
#include <boost/bind.hpp>
#include <list>
#include <stdio.h>
#include <unistd.h>

using namespace muduo;
using namespace muduo::net;

// RFC 862
class EchoServer
{
 public:
  EchoServer(EventLoop* loop,
             const InetAddress& listenAddr,
             int idleSeconds);

  void start()
  {
    server_.start();
  }

 private:
  void onConnection(const TcpConnectionPtr& conn);

  void onMessage(const TcpConnectionPtr& conn,
                 Buffer* buf,
                 Timestamp time);

  void onTimer();

  void dumpConnectionList() const;

  typedef boost::weak_ptr<TcpConnection> WeakTcpConnectionPtr;
  typedef std::list<WeakTcpConnectionPtr> WeakConnectionList;

  struct Node : public muduo::copyable
  {
    Timestamp lastReceiveTime;
    WeakConnectionList::iterator position;
  };

  TcpServer server_;
  int idleSeconds_;
  WeakConnectionList connectionList_;
};

EchoServer::EchoServer(EventLoop* loop,
                       const InetAddress& listenAddr,
                       int idleSeconds)
  : server_(loop, listenAddr, "EchoServer"),
    idleSeconds_(idleSeconds)
{
  server_.setConnectionCallback(
      boost::bind(&EchoServer::onConnection, this, _1));
  server_.setMessageCallback(
      boost::bind(&EchoServer::onMessage, this, _1, _2, _3));
  loop->runEvery(1.0, boost::bind(&EchoServer::onTimer, this));
  dumpConnectionList();
}

void EchoServer::onConnection(const TcpConnectionPtr& conn)
{
  LOG_INFO << "EchoServer - " << conn->peerAddress().toIpPort() << " -> "
           << conn->localAddress().toIpPort() << " is "
           << (conn->connected() ? "UP" : "DOWN");

  if (conn->connected())
  {
    Node node;
    node.lastReceiveTime = Timestamp::now();
    connectionList_.push_back(conn);
    node.position = --connectionList_.end();
    conn->setContext(node);
  }
  else
  {
    assert(!conn->getContext().empty());
    const Node& node = boost::any_cast<const Node&>(conn->getContext());
    connectionList_.erase(node.position);
  }
  dumpConnectionList();
}

void EchoServer::onMessage(const TcpConnectionPtr& conn,
                           Buffer* buf,
                           Timestamp time)
{
  string msg(buf->retrieveAllAsString());
  LOG_INFO << conn->name() << " echo " << msg.size()
           << " bytes at " << time.toString();
  conn->send(msg);

  assert(!conn->getContext().empty());
  Node* node = boost::any_cast<Node>(conn->getMutableContext());
  node->lastReceiveTime = time;
  connectionList_.splice(connectionList_.end(), connectionList_, node->position);
  assert(node->position == --connectionList_.end());

  dumpConnectionList();
}

void EchoServer::onTimer()
{
  dumpConnectionList();
  Timestamp now = Timestamp::now();
  for (WeakConnectionList::iterator it = connectionList_.begin();
      it != connectionList_.end();)
  {
    TcpConnectionPtr conn = it->lock();
    if (conn)
    {
      Node* n = boost::any_cast<Node>(conn->getMutableContext());
      double age = timeDifference(now, n->lastReceiveTime);
      if (age > idleSeconds_)
      {
        if (conn->connected())
        {
          conn->shutdown();
          LOG_INFO << "shutting down " << conn->name();
          conn->forceCloseWithDelay(3.5);  // > round trip of the whole Internet.
        }
      }
      else if (age < 0)
      {
        LOG_WARN << "Time jump";
        n->lastReceiveTime = now;
      }
      else
      {
        break;
      }
      ++it;
    }
    else
    {
      LOG_WARN << "Expired";
      it = connectionList_.erase(it);
    }
  }
}

void EchoServer::dumpConnectionList() const
{
  LOG_INFO << "size = " << connectionList_.size();

  for (WeakConnectionList::const_iterator it = connectionList_.begin();
      it != connectionList_.end(); ++it)
  {
    TcpConnectionPtr conn = it->lock();
    if (conn)
    {
      printf("conn %p\n", get_pointer(conn));
      const Node& n = boost::any_cast<const Node&>(conn->getContext());
      printf("    time %s\n", n.lastReceiveTime.toString().c_str());
    }
    else
    {
      printf("expired\n");
    }
  }
}

int main(int argc, char* argv[])
{
  EventLoop loop;
  InetAddress listenAddr(2007);
  int idleSeconds = 10;
  if (argc > 1)
  {
    idleSeconds = atoi(argv[1]);
  }
  LOG_INFO << "pid = " << getpid() << ", idle seconds = " << idleSeconds;
  EchoServer server(&loop, listenAddr, idleSeconds);
  server.start();
  loop.loop();
}

