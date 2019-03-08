#ifndef MUDUO_EXAMPLES_MEMCACHED_SERVER_MEMCACHESERVER_H
#define MUDUO_EXAMPLES_MEMCACHED_SERVER_MEMCACHESERVER_H

#include "examples/memcached/server/Item.h"
#include "examples/memcached/server/Session.h"

#include "muduo/base/Mutex.h"
#include "muduo/net/TcpServer.h"
#include "examples/wordcount/hash.h"

#include <array>
#include <unordered_map>
#include <unordered_set>

class MemcacheServer : muduo::noncopyable
{
 public:
  struct Options
  {
    Options();
    uint16_t tcpport;
    uint16_t udpport;
    uint16_t gperfport;
    int threads;
  };

  MemcacheServer(muduo::net::EventLoop* loop, const Options&);
  ~MemcacheServer();

  void setThreadNum(int threads) { server_.setThreadNum(threads); }
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
  std::unordered_map<string, SessionPtr> sessions_ GUARDED_BY(mutex_);

  // a complicated solution to save memory
  struct Hash
  {
    size_t operator()(const ConstItemPtr& x) const
    {
      return x->hash();
    }
  };

  struct Equal
  {
    bool operator()(const ConstItemPtr& x, const ConstItemPtr& y) const
    {
      return x->key() == y->key();
    }
  };

  typedef std::unordered_set<ConstItemPtr, Hash, Equal> ItemMap;

  struct MapWithLock
  {
    ItemMap items;
    mutable muduo::MutexLock mutex;
  };

  const static int kShards = 4096;

  std::array<MapWithLock, kShards> shards_;

  // NOT guarded by mutex_, but here because server_ has to destructs before
  // sessions_
  muduo::net::TcpServer server_;
  std::unique_ptr<Stats> stats_ PT_GUARDED_BY(mutex_);
};

#endif  // MUDUO_EXAMPLES_MEMCACHED_SERVER_MEMCACHESERVER_H
