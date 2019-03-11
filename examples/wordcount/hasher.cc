#include "muduo/base/CountDownLatch.h"
#include "muduo/base/Logging.h"
#include "muduo/net/EventLoopThread.h"
#include "muduo/net/TcpClient.h"

#include <boost/tokenizer.hpp>

#include "examples/wordcount/hash.h"

#include <fstream>
#include <iostream>

#include <stdio.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

using namespace muduo;
using namespace muduo::net;

size_t g_batchSize = 65536;
const size_t kMaxHashSize = 10 * 1000 * 1000;

class SendThrottler : muduo::noncopyable
{
 public:
  SendThrottler(EventLoop* loop, const InetAddress& addr)
    : client_(loop, addr, "Sender"),
      connectLatch_(1),
      disconnectLatch_(1),
      cond_(mutex_),
      congestion_(false)
  {
    LOG_INFO << "SendThrottler [" << addr.toIpPort() << "]";
    client_.setConnectionCallback(
        std::bind(&SendThrottler::onConnection, this, _1));
  }

  void connect()
  {
    client_.connect();
    connectLatch_.wait();
  }

  void disconnect()
  {
    if (buffer_.readableBytes() > 0)
    {
      LOG_DEBUG << "send " << buffer_.readableBytes() << " bytes";
      conn_->send(&buffer_);
    }
    conn_->shutdown();
    disconnectLatch_.wait();
  }

  void send(const string& word, int64_t count)
  {
    buffer_.append(word);
    // FIXME: use LogStream
    char buf[64];
    snprintf(buf, sizeof buf, "\t%" PRId64 "\r\n", count);
    buffer_.append(buf);
    if (buffer_.readableBytes() >= g_batchSize)
    {
      throttle();
      LOG_TRACE << "send " << buffer_.readableBytes();
      conn_->send(&buffer_);
    }
  }

 private:
  void onConnection(const TcpConnectionPtr& conn)
  {
    if (conn->connected())
    {
      conn->setHighWaterMarkCallback(
          std::bind(&SendThrottler::onHighWaterMark, this), 1024*1024);
      conn->setWriteCompleteCallback(
          std::bind(&SendThrottler::onWriteComplete, this));

      conn_ = conn;
      connectLatch_.countDown();
    }
    else
    {
      conn_.reset();
      disconnectLatch_.countDown();
    }
  }

  void onHighWaterMark()
  {
    MutexLockGuard lock(mutex_);
    congestion_ = true;
  }

  void onWriteComplete()
  {
    MutexLockGuard lock(mutex_);
    bool oldCong = congestion_;
    congestion_ = false;
    if (oldCong)
    {
      cond_.notify();
    }
  }

  void throttle()
  {
    MutexLockGuard lock(mutex_);
    while (congestion_)
    {
      LOG_DEBUG << "wait ";
      cond_.wait();
    }
  }

  TcpClient client_;
  TcpConnectionPtr conn_;
  CountDownLatch connectLatch_;
  CountDownLatch disconnectLatch_;
  Buffer buffer_;

  MutexLock mutex_;
  Condition cond_;
  bool congestion_;
};

class WordCountSender : muduo::noncopyable
{
 public:
  explicit WordCountSender(const std::string& receivers);

  void connectAll()
  {
    for (size_t i = 0; i < buckets_.size(); ++i)
    {
      buckets_[i]->connect();
    }
    LOG_INFO << "All connected";
  }

  void disconnectAll()
  {
    for (size_t i = 0; i < buckets_.size(); ++i)
    {
      buckets_[i]->disconnect();
    }
    LOG_INFO << "All disconnected";
  }

  void processFile(const char* filename);

 private:
  EventLoopThread loopThread_;
  EventLoop* loop_;
  std::vector<std::unique_ptr<SendThrottler>> buckets_;
};

WordCountSender::WordCountSender(const std::string& receivers)
  : loop_(loopThread_.startLoop())
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(", ");
  tokenizer tokens(receivers, sep);
  for (tokenizer::iterator tok_iter = tokens.begin();
       tok_iter != tokens.end(); ++tok_iter)
  {
    std::string ipport = *tok_iter;
    size_t colon = ipport.find(':');
    if (colon != std::string::npos)
    {
      uint16_t port = static_cast<uint16_t>(atoi(&ipport[colon+1]));
      InetAddress addr(ipport.substr(0, colon), port);
      buckets_.emplace_back(new SendThrottler(loop_, addr));
    }
    else
    {
      assert(0 && "Invalid address");
    }
  }
}

void WordCountSender::processFile(const char* filename)
{
  LOG_INFO << "processFile " << filename;
  WordCountMap wordcounts;
  // FIXME: use mmap to read file
  std::ifstream in(filename);
  string word;
  // FIXME: make local hash optional.
  std::hash<string> hash;
  while (in)
  {
    wordcounts.clear();
    while (in >> word)
    {
      wordcounts[word] += 1;
      if (wordcounts.size() > kMaxHashSize)
      {
        break;
      }
    }

    LOG_INFO << "send " << wordcounts.size() << " records";
    for (WordCountMap::iterator it = wordcounts.begin();
         it != wordcounts.end(); ++it)
    {
      size_t idx = hash(it->first) % buckets_.size();
      buckets_[idx]->send(it->first, it->second);
    }
  }
}

int main(int argc, char* argv[])
{
  if (argc < 3)
  {
    printf("Usage: %s addresses_of_receivers input_file1 [input_file2]* \n", argv[0]);
    printf("Example: %s 'ip1:port1,ip2:port2,ip3:port3' input_file1 input_file2 \n", argv[0]);
  }
  else
  {
    const char* batchSize = ::getenv("BATCH_SIZE");
    if (batchSize)
    {
      g_batchSize = atoi(batchSize);
    }
    WordCountSender sender(argv[1]);
    sender.connectAll();
    for (int i = 2; i < argc; ++i)
    {
      sender.processFile(argv[i]);
    }
    sender.disconnectAll();
  }
}
