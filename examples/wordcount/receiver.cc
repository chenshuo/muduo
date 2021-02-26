#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>

#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>

#include <examples/wordcount/hash.h>

#include <fstream>

#include <stdio.h>

using namespace muduo;
using namespace muduo::net;

class WordCountReceiver : boost::noncopyable
{
 public:
  WordCountReceiver(EventLoop* loop, const InetAddress& listenAddr)
    : loop_(loop),
      server_(loop, listenAddr, "WordCountReceiver"),
      senders_(0)
  {
    server_.setConnectionCallback(
         boost::bind(&WordCountReceiver::onConnection, this, _1));
    server_.setMessageCallback(
        boost::bind(&WordCountReceiver::onMessage, this, _1, _2, _3));
  }

  void start(int senders)
  {
    LOG_INFO << "start " << senders << " senders";
    senders_ = senders;
    wordcounts_.clear();
    server_.start();
  }

 private:
  void onConnection(const TcpConnectionPtr& conn)
  {
    LOG_DEBUG << conn->peerAddress().toIpPort() << " -> "
              << conn->localAddress().toIpPort() << " is "
              << (conn->connected() ? "UP" : "DOWN");
    if (!conn->connected())
    {
      if (--senders_ == 0)
      {
        output();
        loop_->quit();
      }
    }
  }

  void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp)
  {
    const char* crlf = NULL;
    while ( (crlf = buf->findCRLF()) != NULL)
    {
      // string request(buf->peek(), crlf);
      // printf("%s\n", request.c_str());
      const char* tab = std::find(buf->peek(), crlf, '\t');
      if (tab != crlf)
      {
        string word(buf->peek(), tab);
        int64_t cnt = atoll(tab);
        wordcounts_[word] += cnt;
      }
      else
      {
        LOG_ERROR << "Wrong format, no tab found";
        conn->shutdown();
      }
      buf->retrieveUntil(crlf + 2);
    }
  }

  void output()
  {
    LOG_INFO << "Writing shard";
    std::ofstream out("shard");
    for (WordCountMap::iterator it = wordcounts_.begin();
         it != wordcounts_.end(); ++it)
    {
      out << it->first << '\t' << it->second << '\n';
    }
  }

  EventLoop* loop_;
  TcpServer server_;
  int senders_;
  WordCountMap wordcounts_;
};

int main(int argc, char* argv[])
{
  if (argc < 3)
  {
    printf("Usage: %s listen_port number_of_senders\n", argv[0]);
  }
  else
  {
    EventLoop loop;
    int port = atoi(argv[1]);
    InetAddress addr(static_cast<uint16_t>(port));
    WordCountReceiver receiver(&loop, addr);
    receiver.start(atoi(argv[2]));
    loop.loop();
  }
}
