#ifndef MUDUO_EXAMPLES_MEMCACHED_SERVER_SESSION_H
#define MUDUO_EXAMPLES_MEMCACHED_SERVER_SESSION_H

#include "examples/memcached/server/Item.h"

#include "muduo/base/Logging.h"

#include "muduo/net/TcpConnection.h"

#include <boost/tokenizer.hpp>

using muduo::string;

class MemcacheServer;

class Session : public std::enable_shared_from_this<Session>,
                muduo::noncopyable
{
 public:
  Session(MemcacheServer* owner, const muduo::net::TcpConnectionPtr& conn)
    : owner_(owner),
      conn_(conn),
      state_(kNewCommand),
      protocol_(kAscii), // FIXME
      noreply_(false),
      policy_(Item::kInvalid),
      bytesToDiscard_(0),
      needle_(Item::makeItem(kLongestKey, 0, 0, 2, 0)),
      bytesRead_(0),
      requestsProcessed_(0)
  {
    using std::placeholders::_1;
    using std::placeholders::_2;
    using std::placeholders::_3;

    conn_->setMessageCallback(
        std::bind(&Session::onMessage, this, _1, _2, _3));
  }

  ~Session()
  {
    LOG_INFO << "requests processed: " << requestsProcessed_
             << " input buffer size: " << conn_->inputBuffer()->internalCapacity()
             << " output buffer size: " << conn_->outputBuffer()->internalCapacity();
  }

 private:
  enum State
  {
    kNewCommand,
    kReceiveValue,
    kDiscardValue,
  };

  enum Protocol
  {
    kAscii,
    kBinary,
    kAuto,
  };

  void onMessage(const muduo::net::TcpConnectionPtr& conn,
                 muduo::net::Buffer* buf,
                 muduo::Timestamp);
  void onWriteComplete(const muduo::net::TcpConnectionPtr& conn);
  void receiveValue(muduo::net::Buffer* buf);
  void discardValue(muduo::net::Buffer* buf);
  // TODO: highWaterMark
  // TODO: onWriteComplete

  // returns true if finished a request
  bool processRequest(muduo::StringPiece request);
  void resetRequest();
  void reply(muduo::StringPiece msg);

  struct SpaceSeparator
  {
    void reset() {}
    template <typename InputIterator, typename Token>
    bool operator()(InputIterator& next, InputIterator end, Token& tok);
  };

  typedef boost::tokenizer<SpaceSeparator,
      const char*,
      muduo::StringPiece> Tokenizer;
  struct Reader;
  bool doUpdate(Tokenizer::iterator& beg, Tokenizer::iterator end);
  void doDelete(Tokenizer::iterator& beg, Tokenizer::iterator end);

  MemcacheServer* owner_;
  muduo::net::TcpConnectionPtr conn_;
  State state_;
  Protocol protocol_;

  // current request
  string command_;
  bool noreply_;
  Item::UpdatePolicy policy_;
  ItemPtr currItem_;
  size_t bytesToDiscard_;
  // cached
  ItemPtr needle_;
  muduo::net::Buffer outputBuf_;

  // per session stats
  size_t bytesRead_;
  size_t requestsProcessed_;

  static string kLongestKey;
};

typedef std::shared_ptr<Session> SessionPtr;

#endif  // MUDUO_EXAMPLES_MEMCACHED_SERVER_SESSION_H
