#ifndef MUDUO_EXAMPLES_MEMCACHED_SERVER_MEMCACHESERVER_H
#define MUDUO_EXAMPLES_MEMCACHED_SERVER_MEMCACHESERVER_H

#include "Item.h"
#include "Session.h"

#include <muduo/net/TcpServer.h>
#include <examples/wordcount/hash.h>

#include <boost/noncopyable.hpp>
#include <boost/unordered_map.hpp>

class MemcacheServer : boost::noncopyable
{
 public:
  struct Options
  {
    Options();
    uint16_t tcpport;
    uint16_t udpport;
    int threads;
  };

  MemcacheServer(muduo::net::EventLoop* loop, const Options&);
  ~MemcacheServer();

  void start();
  void stop();

  bool storeItem(const ItemPtr& item, Item::UpdatePolicy policy, bool* exists);
  ConstItemPtr getItem(const string& key) const;
  bool deleteItem(const string& key);

 private:
  void onConnection(const muduo::net::TcpConnectionPtr& conn);

  struct Stats;

  muduo::net::EventLoop* loop_;  // not own
  Options options_;

  mutable muduo::MutexLock mutex_;
  boost::unordered_map<string, SessionPtr> sessions_;

  boost::unordered_map<string, ConstItemPtr> items_;

  muduo::net::TcpServer server_;
  boost::scoped_ptr<Stats> stats_;
};

#endif  // MUDUO_EXAMPLES_MEMCACHED_SERVER_MEMCACHESERVER_H
