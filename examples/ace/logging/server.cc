#include "examples/ace/logging/logrecord.pb.h"

#include "muduo/base/Atomic.h"
#include "muduo/base/FileUtil.h"
#include "muduo/base/Logging.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/TcpServer.h"
#include "muduo/net/protobuf/ProtobufCodecLite.h"

#include <stdio.h>

using namespace muduo;
using namespace muduo::net;

namespace logging
{
extern const char logtag[] = "LOG0";
typedef ProtobufCodecLiteT<LogRecord, logtag> Codec;

class Session : noncopyable
{
 public:
  explicit Session(const TcpConnectionPtr& conn)
    : codec_(std::bind(&Session::onMessage, this, _1, _2, _3)),
      file_(getFileName(conn))
  {
    conn->setMessageCallback(
        std::bind(&Codec::onMessage, &codec_, _1, _2, _3));
  }

 private:

  // FIXME: duplicate code LogFile
  // or use LogFile instead
  string getFileName(const TcpConnectionPtr& conn)
  {
    string filename;
    filename += conn->peerAddress().toIp();

    char timebuf[32];
    struct tm tm;
    time_t now = time(NULL);
    gmtime_r(&now, &tm); // FIXME: localtime_r ?
    strftime(timebuf, sizeof timebuf, ".%Y%m%d-%H%M%S.", &tm);
    filename += timebuf;

    char buf[32];
    snprintf(buf, sizeof buf, "%d", globalCount_.incrementAndGet());
    filename += buf;

    filename += ".log";
    LOG_INFO << "Session of " << conn->name() << " file " << filename;
    return filename;
  }

  void onMessage(const TcpConnectionPtr& conn,
                 const MessagePtr& message,
                 Timestamp time)
  {
    LogRecord* logRecord = muduo::down_cast<LogRecord*>(message.get());
    if (logRecord->has_heartbeat())
    {
      // FIXME ?
    }
    const char* sep = "==========\n";
    std::string str = logRecord->DebugString();
    file_.append(str.c_str(), str.size());
    file_.append(sep, strlen(sep));
    LOG_DEBUG << str;
  }

  Codec codec_;
  FileUtil::AppendFile file_;
  static AtomicInt32 globalCount_;
};
typedef std::shared_ptr<Session> SessionPtr;

AtomicInt32 Session::globalCount_;

class LogServer : noncopyable
{
 public:
  LogServer(EventLoop* loop, const InetAddress& listenAddr, int numThreads)
    : loop_(loop),
      server_(loop_, listenAddr, "AceLoggingServer")
  {
    server_.setConnectionCallback(
        std::bind(&LogServer::onConnection, this, _1));
    if (numThreads > 1)
    {
      server_.setThreadNum(numThreads);
    }
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
      SessionPtr session(new Session(conn));
      conn->setContext(session);
    }
    else
    {
      conn->setContext(SessionPtr());
    }
  }

  EventLoop* loop_;
  TcpServer server_;
};

}  // namespace logging

int main(int argc, char* argv[])
{
  EventLoop loop;
  int port = argc > 1 ? atoi(argv[1]) : 50000;
  LOG_INFO << "Listen on port " << port;
  InetAddress listenAddr(static_cast<uint16_t>(port));
  int numThreads = argc > 2 ? atoi(argv[2]) : 1;
  logging::LogServer server(&loop, listenAddr, numThreads);
  server.start();
  loop.loop();

}
