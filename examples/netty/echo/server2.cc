#include <muduo/net/TcpServer.h>

#include <muduo/base/Atomic.h>
#include <muduo/base/FileUtil.h>
#include <muduo/base/Logging.h>
#include <muduo/base/ProcessInfo.h>
#include <muduo/base/Thread.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>

#include <boost/bind.hpp>

#include <utility>

#include <stdio.h>
#include <unistd.h>

using namespace muduo;
using namespace muduo::net;

int numThreads = 0;

class EchoServer
{
 public:
  EchoServer(EventLoop* loop, const InetAddress& listenAddr)
    : server_(loop, listenAddr, "EchoServer"),
      startTime_(Timestamp::now())
  {
    server_.setConnectionCallback(
        boost::bind(&EchoServer::onConnection, this, _1));
    server_.setMessageCallback(
        boost::bind(&EchoServer::onMessage, this, _1, _2, _3));
    server_.setThreadNum(numThreads);
    loop->runEvery(5.0, boost::bind(&EchoServer::printThroughput, this));
  }

  void start()
  {
    LOG_INFO << "starting " << numThreads << " threads.";
    server_.start();
  }

 private:
  void onConnection(const TcpConnectionPtr& conn)
  {
    LOG_TRACE << conn->peerAddress().toIpPort() << " -> "
        << conn->localAddress().toIpPort() << " is "
        << (conn->connected() ? "UP" : "DOWN");
    conn->setTcpNoDelay(true);
    if (conn->connected())
    {
      connections_.increment();
    }
    else
    {
      connections_.decrement();
    }
  }

  void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp)
  {
    size_t len = buf->readableBytes();
    transferredBytes_.addAndGet(len);
    receivedMessages_.incrementAndGet();
    conn->send(buf);
  }

  void printThroughput()
  {
    Timestamp endTime = Timestamp::now();
    double bytes = static_cast<double>(transferredBytes_.getAndSet(0));
    int msgs = receivedMessages_.getAndSet(0);
    double bytesPerMsg = msgs > 0 ?  bytes/msgs : 0;
    double time = timeDifference(endTime, startTime_);
    printf("%.3f MiB/s %.2f Kilo Msgs/s %.2f bytes per msg, ",
        bytes/time/1024/1024,
        static_cast<double>(msgs)/time/1000,
        bytesPerMsg);

    printConnection();
    fflush(stdout);
    startTime_ = endTime;
  }

  void printConnection()
  {
    string procStatus = ProcessInfo::procStatus();
    printf("%d conn, files %d , VmSize %ld KiB, RSS %ld KiB, ",
           connections_.get(),
           ProcessInfo::openedFiles(),
           getLong(procStatus, "VmSize:"),
           getLong(procStatus, "VmRSS:"));

    string meminfo;
    FileUtil::readFile("/proc/meminfo", 65536, &meminfo);
    long total_kb = getLong(meminfo, "MemTotal:");
    long free_kb = getLong(meminfo, "MemFree:");
    long buffers_kb = getLong(meminfo, "Buffers:");
    long cached_kb = getLong(meminfo, "Cached:");
    printf("system memory used %ld KiB\n",
           total_kb - free_kb - buffers_kb - cached_kb);
  }

  long getLong(const string& procStatus, const char* key)
  {
    long result = 0;
    size_t pos = procStatus.find(key);
    if (pos != string::npos)
    {
      result = ::atol(procStatus.c_str() + pos + strlen(key));
    }
    return result;
  }

  TcpServer server_;
  AtomicInt32 connections_;
  AtomicInt32 receivedMessages_;
  AtomicInt64 transferredBytes_;
  Timestamp startTime_;
};

int main(int argc, char* argv[])
{
  LOG_INFO << "pid = " << getpid()
           << ", tid = " << CurrentThread::tid()
           << ", max files = " << ProcessInfo::maxOpenFiles();
  Logger::setLogLevel(Logger::WARN);
  if (argc > 1)
  {
    numThreads = atoi(argv[1]);
  }
  EventLoop loop;
  InetAddress listenAddr(2007);
  EchoServer server(&loop, listenAddr);

  server.start();

  loop.loop();
}

