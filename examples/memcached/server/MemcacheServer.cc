#include "MemcacheServer.h"

#include <muduo/base/Atomic.h>
#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>

#include <boost/bind.hpp>

using namespace muduo;
using namespace muduo::net;

muduo::AtomicInt64 g_cas;

MemcacheServer::Options::Options()
{
  bzero(this, sizeof(*this));
}

struct MemcacheServer::Stats
{
};

MemcacheServer::MemcacheServer(muduo::net::EventLoop* loop, const Options& options)
  : loop_(loop),
    options_(options),
    server_(loop, InetAddress(options.tcpport), "muduo-memcached"),
    stats_(new Stats)
{
  server_.setConnectionCallback(
      boost::bind(&MemcacheServer::onConnection, this, _1));
}

MemcacheServer::~MemcacheServer()
{
}

void MemcacheServer::start()
{
  server_.start();
}

void MemcacheServer::stop()
{
  loop_->runAfter(3.0, boost::bind(&EventLoop::quit, loop_));
}

bool MemcacheServer::storeItem(const ItemPtr& item, const Item::UpdatePolicy policy, bool* exists)
{
  assert(item->neededBytes() == 0);
  MutexLockGuard lock(mutex_);
  if (policy == Item::kSet)
  {
    item->setCas(g_cas.incrementAndGet());
    items_[item->key()] = item;
  }
  else
  {
    boost::unordered_map<string, ConstItemPtr>::iterator it = items_.find(item->key());
    *exists = it != items_.end();
    if (policy == Item::kAdd)
    {
      if (*exists)
      {
        return false;
      }
      else
      {
        item->setCas(g_cas.incrementAndGet());
        items_[item->key()] = item;
      }
    }
    else if (policy == Item::kReplace)
    {
      if (*exists)
      {
        item->setCas(g_cas.incrementAndGet());
        it->second = item;
      }
      else
      {
        return false;
      }
    }
    else if (policy == Item::kAppend || policy == Item::kPrepend)
    {
      if (*exists)
      {
        const ConstItemPtr& oldItem = it->second;
        int newLen = static_cast<int>(item->valueLength() + oldItem->valueLength() - 2);
        ItemPtr newItem(new Item(item->key(),
                                 item->flags(),
                                 item->exptime(),
                                 newLen,
                                 g_cas.incrementAndGet()));
        if (policy == Item::kAppend)
        {
          newItem->append(oldItem->value(), oldItem->valueLength() - 2);
          newItem->append(item->value(), item->valueLength());
        }
        else
        {
          newItem->append(item->value(), item->valueLength() - 2);
          newItem->append(oldItem->value(), oldItem->valueLength());
        }
        assert(newItem->neededBytes() == 0);
        assert(newItem->endsWithCRLF());
        it->second = newItem;
      }
      else
      {
        return false;
      }
    }
    else if (policy == Item::kCas)
    {
      if (*exists && it->second->cas() == item->cas())
      {
        item->setCas(g_cas.incrementAndGet());
        it->second = item;
      }
      else
      {
        return false;
      }
    }
    else
    {
      assert(false);
    }
  }
  return true;
}

ConstItemPtr MemcacheServer::getItem(const string& key) const
{
  MutexLockGuard lock(mutex_);
  boost::unordered_map<string, ConstItemPtr>::const_iterator it = items_.find(key);
  return it != items_.end() ? it->second : ConstItemPtr();
}

bool MemcacheServer::deleteItem(const string& key)
{
  MutexLockGuard lock(mutex_);
  return items_.erase(key) == 1;
}

void MemcacheServer::onConnection(const TcpConnectionPtr& conn)
{
  if (conn->connected())
  {
    SessionPtr session(new Session(this, conn));
    MutexLockGuard lock(mutex_);
    assert(sessions_.find(conn->name()) == sessions_.end());
    sessions_[conn->name()] = session;
    // assert(sessions_.size() == stats_.current_conns);
  }
  else
  {
    MutexLockGuard lock(mutex_);
    assert(sessions_.find(conn->name()) != sessions_.end());
    sessions_.erase(conn->name());
    // assert(sessions_.size() == stats_.current_conns);
  }
}
