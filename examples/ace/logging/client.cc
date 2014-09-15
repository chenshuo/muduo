#include <examples/ace/logging/logrecord.pb.h>

#include <muduo/base/Mutex.h>
#include <muduo/base/Logging.h>
#include <muduo/base/ProcessInfo.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/EventLoopThread.h>
#include <muduo/net/TcpClient.h>
#include <muduo/net/protobuf/ProtobufCodecLite.h>

#include <boost/bind.hpp>

#include <iostream>
#include <stdio.h>

using namespace muduo;
using namespace muduo::net;

// just to verify the protocol, not for practical usage.

namespace logging
{
extern const char logtag[] = "LOG0";
typedef ProtobufCodecLiteT<LogRecord, logtag> Codec;

// same as asio/char/client.cc
class LogClient : boost::noncopyable
{
 public:
  LogClient(EventLoop* loop, const InetAddress& serverAddr)
    : client_(loop, serverAddr, "LogClient"),
      codec_(boost::bind(&LogClient::onMessage, this, _1, _2, _3))
  {
    client_.setConnectionCallback(
        boost::bind(&LogClient::onConnection, this, _1));
    client_.setMessageCallback(
        boost::bind(&Codec::onMessage, &codec_, _1, _2, _3));
    client_.enableRetry();
  }

  void connect()
  {
    client_.connect();
  }

  void disconnect()
  {
    client_.disconnect();
  }

  void write(const StringPiece& message)
  {
    MutexLockGuard lock(mutex_);
    updateLogRecord(message);
    if (connection_)
    {
      codec_.send(connection_, logRecord_);
    }
    else
    {
      LOG_WARN << "NOT CONNECTED";
    }
  }

 private:
  void onConnection(const TcpConnectionPtr& conn)
  {
    LOG_INFO << conn->localAddress().toIpPort() << " -> "
             << conn->peerAddress().toIpPort() << " is "
             << (conn->connected() ? "UP" : "DOWN");

    MutexLockGuard lock(mutex_);
    if (conn->connected())
    {
      connection_ = conn;
      LogRecord_Heartbeat* hb = logRecord_.mutable_heartbeat();
      hb->set_hostname(ProcessInfo::hostname().c_str());
      hb->set_process_name(ProcessInfo::procname().c_str());
      hb->set_process_id(ProcessInfo::pid());
      hb->set_process_start_time(ProcessInfo::startTime().microSecondsSinceEpoch());
      hb->set_username(ProcessInfo::username().c_str());
      updateLogRecord("Heartbeat");
      codec_.send(connection_, logRecord_);
      logRecord_.clear_heartbeat();
      LOG_INFO << "Type message below:";
    }
    else
    {
      connection_.reset();
    }
  }

  void onMessage(const TcpConnectionPtr&,
                 const MessagePtr& message,
                 Timestamp)
  {
    // SHOULD NOT HAPPEN
    LogRecord* logRecord = muduo::down_cast<LogRecord*>(message.get());
    LOG_WARN << logRecord->DebugString();
  }

  void updateLogRecord(const StringPiece& message)
  {
    mutex_.assertLocked();
    logRecord_.set_level(1);
    logRecord_.set_thread_id(CurrentThread::tid());
    logRecord_.set_timestamp(Timestamp::now().microSecondsSinceEpoch());
    logRecord_.set_message(message.data(), message.size());
  }

  TcpClient client_;
  Codec codec_;
  MutexLock mutex_;
  LogRecord logRecord_;
  TcpConnectionPtr connection_;
};

}

int main(int argc, char* argv[])
{
  if (argc < 3)
  {
    printf("usage: %s server_ip server_port\n", argv[0]);
  }
  else
  {
    EventLoopThread loopThread;
    uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
    InetAddress serverAddr(argv[1], port);

    logging::LogClient client(loopThread.startLoop(), serverAddr);
    client.connect();
    std::string line;
    while (std::getline(std::cin, line))
    {
      client.write(line);
    }
    client.disconnect();
    CurrentThread::sleepUsec(1000*1000);  // wait for disconnect, then safe to destruct LogClient (esp. TcpClient). Otherwise mutex_ is used after dtor.
  }
  google::protobuf::ShutdownProtobufLibrary();
}
