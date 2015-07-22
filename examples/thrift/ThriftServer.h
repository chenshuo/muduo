#ifndef MUDUO_EXAMPLES_THRIFT_THRIFTSERVER_H
#define MUDUO_EXAMPLES_THRIFT_THRIFTSERVER_H

#include <map>

#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>

#include <muduo/base/ThreadPool.h>
#include <muduo/net/TcpServer.h>

#include <thrift/server/TServer.h>

#include "ThriftConnection.h"

using apache::thrift::TProcessor;
using apache::thrift::TProcessorFactory;
using apache::thrift::protocol::TProtocolFactory;
using apache::thrift::server::TServer;
using apache::thrift::transport::TTransportFactory;

class ThriftServer : boost::noncopyable,
                     public TServer
{
 public:
  template <typename ProcessorFactory>
  ThriftServer(const boost::shared_ptr<ProcessorFactory>& processorFactory,
               muduo::net::EventLoop* eventloop,
               const muduo::net::InetAddress& addr,
               const muduo::string& name,
               THRIFT_OVERLOAD_IF(ProcessorFactory, TProcessorFactory))
    : TServer(processorFactory),
      server_(eventloop, addr, name),
      numWorkerThreads_(0),
      workerThreadPool_(name + muduo::string("WorkerThreadPool"))
  {
    server_.setConnectionCallback(boost::bind(&ThriftServer::onConnection,
                                              this, _1));
  }

  template <typename Processor>
  ThriftServer(const boost::shared_ptr<Processor>& processor,
               muduo::net::EventLoop* eventloop,
               const muduo::net::InetAddress& addr,
               const muduo::string& name,
               THRIFT_OVERLOAD_IF(Processor, TProcessor))
    : TServer(processor),
      server_(eventloop, addr, name),
      numWorkerThreads_(0),
      workerThreadPool_(name + muduo::string("WorkerThreadPool"))
  {
    server_.setConnectionCallback(boost::bind(&ThriftServer::onConnection,
                                              this, _1));
  }

  template <typename ProcessorFactory>
  ThriftServer(const boost::shared_ptr<ProcessorFactory>& processorFactory,
               const boost::shared_ptr<TProtocolFactory>& protocolFactory,
               muduo::net::EventLoop* eventloop,
               const muduo::net::InetAddress& addr,
               const muduo::string& name,
               THRIFT_OVERLOAD_IF(ProcessorFactory, TProcessorFactory))
    : TServer(processorFactory),
      server_(eventloop, addr, name),
      numWorkerThreads_(0),
      workerThreadPool_(name + muduo::string("WorkerThreadPool"))
  {
    server_.setConnectionCallback(boost::bind(&ThriftServer::onConnection,
                                              this, _1));
    setInputProtocolFactory(protocolFactory);
    setOutputProtocolFactory(protocolFactory);
  }

  template <typename Processor>
  ThriftServer(const boost::shared_ptr<Processor>& processor,
               const boost::shared_ptr<TProtocolFactory>& protocolFactory,
               muduo::net::EventLoop* eventloop,
               const muduo::net::InetAddress& addr,
               const muduo::string& name,
               THRIFT_OVERLOAD_IF(Processor, TProcessor))
    : TServer(processor),
      server_(eventloop, addr, name),
      numWorkerThreads_(0),
      workerThreadPool_(name + muduo::string("WorkerThreadPool"))
  {
    server_.setConnectionCallback(boost::bind(&ThriftServer::onConnection,
                                              this, _1));
    setInputProtocolFactory(protocolFactory);
    setOutputProtocolFactory(protocolFactory);
  }

  template <typename ProcessorFactory>
  ThriftServer(const boost::shared_ptr<ProcessorFactory>& processorFactory,
               const boost::shared_ptr<TTransportFactory>& transportFactory,
               const boost::shared_ptr<TProtocolFactory>& protocolFactory,
               muduo::net::EventLoop* eventloop,
               const muduo::net::InetAddress& addr,
               const muduo::string& name,
               THRIFT_OVERLOAD_IF(ProcessorFactory, TProcessorFactory))
    : TServer(processorFactory),
      server_(eventloop, addr, name),
      numWorkerThreads_(0),
      workerThreadPool_(name + muduo::string("WorkerThreadPool"))
  {
    server_.setConnectionCallback(boost::bind(&ThriftServer::onConnection,
                                              this, _1));
    setInputTransportFactory(transportFactory);
    setOutputTransportFactory(transportFactory);
    setInputProtocolFactory(protocolFactory);
    setOutputProtocolFactory(protocolFactory);
  }

  template <typename Processor>
  ThriftServer(const boost::shared_ptr<Processor>& processor,
               const boost::shared_ptr<TTransportFactory>& transportFactory,
               const boost::shared_ptr<TProtocolFactory>& protocolFactory,
               muduo::net::EventLoop* eventloop,
               const muduo::net::InetAddress& addr,
               const muduo::string& name,
               THRIFT_OVERLOAD_IF(Processor, TProcessor))
    : TServer(processor),
      server_(eventloop, addr, name),
      numWorkerThreads_(0),
      workerThreadPool_(name + muduo::string("WorkerThreadPool"))
  {
    server_.setConnectionCallback(boost::bind(&ThriftServer::onConnection,
                                              this, _1));
    setInputTransportFactory(transportFactory);
    setOutputTransportFactory(transportFactory);
    setInputProtocolFactory(protocolFactory);
    setOutputProtocolFactory(protocolFactory);
  }

  template <typename ProcessorFactory>
  ThriftServer(const boost::shared_ptr<ProcessorFactory>& processorFactory,
               const boost::shared_ptr<TTransportFactory>& inputTransportFactory,
               const boost::shared_ptr<TTransportFactory>& outputTransportFactory,
               const boost::shared_ptr<TProtocolFactory>& inputProtocolFactory,
               const boost::shared_ptr<TProtocolFactory>& outputProtocolFactory,
               muduo::net::EventLoop* eventloop,
               const muduo::net::InetAddress& addr,
               const muduo::string& name,
               THRIFT_OVERLOAD_IF(ProcessorFactory, TProcessorFactory))
    : TServer(processorFactory),
      server_(eventloop, addr, name),
      numWorkerThreads_(0),
      workerThreadPool_(name + muduo::string("WorkerThreadPool"))
  {
    server_.setConnectionCallback(boost::bind(&ThriftServer::onConnection,
                                              this, _1));
    setInputTransportFactory(inputTransportFactory);
    setOutputTransportFactory(outputTransportFactory);
    setInputProtocolFactory(inputProtocolFactory);
    setOutputProtocolFactory(outputProtocolFactory);
  }

  template <typename Processor>
  ThriftServer(const boost::shared_ptr<Processor>& processor,
               const boost::shared_ptr<TTransportFactory>& inputTransportFactory,
               const boost::shared_ptr<TTransportFactory>& outputTransportFactory,
               const boost::shared_ptr<TProtocolFactory>& inputProtocolFactory,
               const boost::shared_ptr<TProtocolFactory>& outputProtocolFactory,
               muduo::net::EventLoop* eventloop,
               const muduo::net::InetAddress& addr,
               const muduo::string& name,
               THRIFT_OVERLOAD_IF(Processor, TProcessor))
    : TServer(processor),
      server_(eventloop, addr, name),
      numWorkerThreads_(0),
      workerThreadPool_(name + muduo::string("WorkerThreadPool"))
  {
    server_.setConnectionCallback(boost::bind(&ThriftServer::onConnection,
                                              this, _1));
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
}; // ThriftServer

#endif // MUDUO_EXAMPLES_THRIFT_THRIFTSERVER_H
