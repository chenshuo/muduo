#ifndef MUDUO_EXAMPLES_MEMCACHED_SERVER_MEMCACHESERVER_H
#define MUDUO_EXAMPLES_MEMCACHED_SERVER_MEMCACHESERVER_H

#include "Item.h"
#include "Session.h"

#include <muduo/net/TcpServer.h>
#include <examples/wordcount/hash.h>

#include <boost/noncopyable.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

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

  time_t startTime() const { return startTime_; }

  bool storeItem(const ItemPtr& item, Item::UpdatePolicy policy, bool* exists);
  ConstItemPtr getItem(const ConstItemPtr& key) const;
  bool deleteItem(const ConstItemPtr& key);

 private:
  void onConnection(const muduo::net::TcpConnectionPtr& conn);

  struct Stats;

  muduo::net::EventLoop* loop_;  // not own
  Options options_;
  const time_t startTime_;

  mutable muduo::MutexLock mutex_;
  boost::unordered_map<string, SessionPtr> sessions_;

  // a complicated solution to save memory
  struct Hash
  {
    size_t operator()(const ConstItemPtr& x) const
    {
      muduo::StringPiece key = x->key();
      return boost::hash_range(key.begin(), key.end());
    }
  };

  struct Equal
  {
    bool operator()(const ConstItemPtr& x, const ConstItemPtr& y) const
    {
      return x->key() == y->key();
    }
  };

  typedef boost::unordered_set<ConstItemPtr, Hash, Equal> ItemMap;
  ItemMap items_;

  // NOT guarded by mutex_, but here because server_ has to destructs before
  // sessions_
  muduo::net::TcpServer server_;
  boost::scoped_ptr<Stats> stats_;
};

#endif  // MUDUO_EXAMPLES_MEMCACHED_SERVER_MEMCACHESERVER_H
