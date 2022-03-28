#ifndef MUDUO_CONTRIB_THRIFT_THRIFTSERVER_H
#define MUDUO_CONTRIB_THRIFT_THRIFTSERVER_H

#include <functional>
#include <map>

#include <boost/noncopyable.hpp>

#include "muduo/base/ThreadPool.h"
#include "muduo/net/TcpServer.h"

#include <thrift/server/TServer.h>

#include "contrib/thrift/ThriftConnection.h"

using apache::thrift::TProcessor;
using apache::thrift::TProcessorFactory;
using apache::thrift::protocol::TProtocolFactory;
using apache::thrift::server::TServer;
using apache::thrift::transport::TTransportFactory;

class ThriftServer : boost::noncopyable,
                     public TServer
{
 public:
  ThriftServer(const boost::shared_ptr<TProcessorFactory>& processorFactory,
               muduo::net::EventLoop* eventloop,
               const muduo::net::InetAddress& addr,
               const muduo::string& name)
    : TServer(processorFactory),
      server_(eventloop, addr, name),
      numWorkerThreads_(0),
      workerThreadPool_(name + muduo::string("WorkerThreadPool"))
  {
    server_.setConnectionCallback(std::bind(&ThriftServer::onConnection,
                                              this, muduo::_1));
  }

  ThriftServer(const boost::shared_ptr<TProcessor>& processor,
               muduo::net::EventLoop* eventloop,
               const muduo::net::InetAddress& addr,
               const muduo::string& name)
    : TServer(processor),
      server_(eventloop, addr, name),
      numWorkerThreads_(0),
      workerThreadPool_(name + muduo::string("WorkerThreadPool"))
  {
    server_.setConnectionCallback(std::bind(&ThriftServer::onConnection,
                                              this, muduo::_1));
  }

  ThriftServer(const boost::shared_ptr<TProcessorFactory>& processorFactory,
               const boost::shared_ptr<TProtocolFactory>& protocolFactory,
               muduo::net::EventLoop* eventloop,
               const muduo::net::InetAddress& addr,
               const muduo::string& name)
    : TServer(processorFactory),
      server_(eventloop, addr, name),
      numWorkerThreads_(0),
      workerThreadPool_(name + muduo::string("WorkerThreadPool"))
  {
    server_.setConnectionCallback(std::bind(&ThriftServer::onConnection,
                                              this, muduo::_1));
    setInputProtocolFactory(protocolFactory);
    setOutputProtocolFactory(protocolFactory);
  }

  ThriftServer(const boost::shared_ptr<TProcessor>& processor,
               const boost::shared_ptr<TProtocolFactory>& protocolFactory,
               muduo::net::EventLoop* eventloop,
               const muduo::net::InetAddress& addr,
               const muduo::string& name)
    : TServer(processor),
      server_(eventloop, addr, name),
      numWorkerThreads_(0),
      workerThreadPool_(name + muduo::string("WorkerThreadPool"))
  {
    server_.setConnectionCallback(std::bind(&ThriftServer::onConnection,
                                              this, muduo::_1));
    setInputProtocolFactory(protocolFactory);
    setOutputProtocolFactory(protocolFactory);
  }

  ThriftServer(const boost::shared_ptr<TProcessorFactory>& processorFactory,
               const boost::shared_ptr<TTransportFactory>& transportFactory,
               const boost::shared_ptr<TProtocolFactory>& protocolFactory,
               muduo::net::EventLoop* eventloop,
               const muduo::net::InetAddress& addr,
               const muduo::string& name)
    : TServer(processorFactory),
      server_(eventloop, addr, name),
      numWorkerThreads_(0),
      workerThreadPool_(name + muduo::string("WorkerThreadPool"))
  {
    server_.setConnectionCallback(std::bind(&ThriftServer::onConnection,
                                              this, muduo::_1));
    setInputTransportFactory(transportFactory);
    setOutputTransportFactory(transportFactory);
    setInputProtocolFactory(protocolFactory);
    setOutputProtocolFactory(protocolFactory);
  }

  ThriftServer(const boost::shared_ptr<TProcessor>& processor,
               const boost::shared_ptr<TTransportFactory>& transportFactory,
               const boost::shared_ptr<TProtocolFactory>& protocolFactory,
               muduo::net::EventLoop* eventloop,
               const muduo::net::InetAddress& addr,
               const muduo::string& name)
    : TServer(processor),
      server_(eventloop, addr, name),
      numWorkerThreads_(0),
      workerThreadPool_(name + muduo::string("WorkerThreadPool"))
  {
    server_.setConnectionCallback(std::bind(&ThriftServer::onConnection,
                                              this, muduo::_1));
    setInputTransportFactory(transportFactory);
    setOutputTransportFactory(transportFactory);
    setInputProtocolFactory(protocolFactory);
    setOutputProtocolFactory(protocolFactory);
  }

  ThriftServer(const boost::shared_ptr<TProcessorFactory>& processorFactory,
               const boost::shared_ptr<TTransportFactory>& inputTransportFactory,
               const boost::shared_ptr<TTransportFactory>& outputTransportFactory,
               const boost::shared_ptr<TProtocolFactory>& inputProtocolFactory,
               const boost::shared_ptr<TProtocolFactory>& outputProtocolFactory,
               muduo::net::EventLoop* eventloop,
               const muduo::net::InetAddress& addr,
               const muduo::string& name)
    : TServer(processorFactory),
      server_(eventloop, addr, name),
      numWorkerThreads_(0),
      workerThreadPool_(name + muduo::string("WorkerThreadPool"))
  {
    server_.setConnectionCallback(std::bind(&ThriftServer::onConnection,
                                              this, muduo::_1));
    setInputTransportFactory(inputTransportFactory);
    setOutputTransportFactory(outputTransportFactory);
    setInputProtocolFactory(inputProtocolFactory);
    setOutputProtocolFactory(outputProtocolFactory);
  }

  ThriftServer(const boost::shared_ptr<TProcessor>& processor,
               const boost::shared_ptr<TTransportFactory>& inputTransportFactory,
               const boost::shared_ptr<TTransportFactory>& outputTransportFactory,
               const boost::shared_ptr<TProtocolFactory>& inputProtocolFactory,
               const boost::shared_ptr<TProtocolFactory>& outputProtocolFactory,
               muduo::net::EventLoop* eventloop,
               const muduo::net::InetAddress& addr,
               const muduo::string& name)
    : TServer(processor),
      server_(eventloop, addr, name),
      numWorkerThreads_(0),
      workerThreadPool_(name + muduo::string("WorkerThreadPool"))
  {
    server_.setConnectionCallback(std::bind(&ThriftServer::onConnection,
                                              this, muduo::_1));
    setInputTransportFactory(inputTransportFactory);
    setOutputTransportFactory(outputTransportFactory);
    setInputProtocolFactory(inputProtocolFactory);
    setOutputProtocolFactory(outputProtocolFactory);
  }

  virtual ~ThriftServer();

  void serve();

  void start();

  void stop();

  muduo::ThreadPool& workerThreadPool()
  {
    return workerThreadPool_;
  }

  bool isWorkerThreadPoolProcessing() const
  {
    return numWorkerThreads_ != 0;
  }

  void setThreadNum(int numThreads)
  {
    server_.setThreadNum(numThreads);
  }

  void setWorkerThreadNum(int numWorkerThreads)
  {
    assert(numWorkerThreads > 0);
    numWorkerThreads_ = numWorkerThreads;
  }

 private:
  friend class ThriftConnection;

  void onConnection(const muduo::net::TcpConnectionPtr& conn);

 private:
  muduo::net::TcpServer server_;
  int numWorkerThreads_;
  muduo::ThreadPool workerThreadPool_;
  muduo::MutexLock mutex_;
  std::map<muduo::string, ThriftConnectionPtr> conns_;
};

#endif  // MUDUO_CONTRIB_THRIFT_THRIFTSERVER_H
