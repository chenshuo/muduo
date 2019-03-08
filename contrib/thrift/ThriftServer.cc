#include "contrib/thrift/ThriftServer.h"

#include <boost/bind.hpp>

#include "muduo/net/EventLoop.h"

using namespace muduo;
using namespace muduo::net;

ThriftServer::~ThriftServer() = default;

void ThriftServer::serve()
{
  start();
}

void ThriftServer::start()
{
  if (numWorkerThreads_ > 0)
  {
    workerThreadPool_.start(numWorkerThreads_);
  }
  server_.start();
}

void ThriftServer::stop()
{
  if (numWorkerThreads_ > 0)
  {
    workerThreadPool_.stop();
  }
  server_.getLoop()->runAfter(3.0, boost::bind(&EventLoop::quit,
                                               server_.getLoop()));
}

void ThriftServer::onConnection(const TcpConnectionPtr& conn)
{
  if (conn->connected())
  {
    ThriftConnectionPtr ptr(new ThriftConnection(this, conn));
    MutexLockGuard lock(mutex_);
    assert(conns_.find(conn->name()) == conns_.end());
    conns_[conn->name()] = ptr;
  }
  else
  {
    MutexLockGuard lock(mutex_);
    assert(conns_.find(conn->name()) != conns_.end());
    conns_.erase(conn->name());
  }
}
