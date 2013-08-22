#include "Session.h"
#include "MemcacheServer.h"

#ifdef HAVE_TCMALLOC
#include <google/malloc_extension.h>
#endif

using namespace muduo;
using namespace muduo::net;

static bool isBinaryProtocol(uint8_t firstByte)
{
  return firstByte == 0x80;
}

const int kLongestKeySize = 250;
string Session::kLongestKey(kLongestKeySize, 'x');

template <typename InputIterator, typename Token>
bool Session::SpaceSeparator::operator()(InputIterator& next, InputIterator end, Token& tok)
{
  while (next != end && *next == ' ')
    ++next;
  if (next == end)
  {
    tok.clear();
    return false;
  }
  InputIterator start(next);
  const char* sp = static_cast<const char*>(memchr(start, ' ', end - start));
  if (sp)
  {
    tok.set(start, static_cast<int>(sp - start));
    next = sp;
  }
  else
  {
    tok.set(start, static_cast<int>(end - next));
    next = end;
  }
  return true;
}

struct Session::Reader
{
  Reader(Tokenizer::iterator& beg, Tokenizer::iterator end)
      : first_(beg),
        last_(end)
  {
  }

  template<typename T>
  bool read(T* val)
  {
    if (first_ == last_)
      return false;
    char* end = NULL;
    uint64_t x = strtoull((*first_).data(), &end, 10);
    if (end == (*first_).end())
    {
      *val = static_cast<T>(x);
      ++first_;
      return true;
    }
    return false;
  }

 private:
  Tokenizer::iterator first_;
  Tokenizer::iterator last_;;
};

void Session::onMessage(const muduo::net::TcpConnectionPtr& conn,
                        muduo::net::Buffer* buf,
                        muduo::Timestamp)

{
  const size_t initialReadable = buf->readableBytes();

  while (buf->readableBytes() > 0)
  {
    if (state_ == kNewCommand)
    {
      if (protocol_ == kAuto)
      {
        assert(bytesRead_ == 0);
        protocol_ = isBinaryProtocol(buf->peek()[0]) ? kBinary : kAscii;
      }

      assert(protocol_ == kAscii || protocol_ == kBinary);
      if (protocol_ == kBinary)
      {
        // FIXME
      }
      else  // ASCII protocol
      {
        const char* crlf = buf->findCRLF();
        if (crlf)
        {
          int len = static_cast<int>(crlf - buf->peek());
          StringPiece request(buf->peek(), len);
          if (processRequest(request))
          {
            resetRequest();
          }
          buf->retrieveUntil(crlf + 2);
        }
        else
        {
          if (buf->readableBytes() > 1024)
          {
            // FIXME: check for 'get' and 'gets'
            conn_->shutdown();
            // buf->retrieveAll() ???
          }
          break;
        }
      }
    }
    else if (state_ == kReceiveValue)
    {
      receiveValue(buf);
    }
    else if (state_ == kDiscardValue)
    {
      discardValue(buf);
    }
    else
    {
      assert(false);
    }
  }
  bytesRead_ += initialReadable - buf->readableBytes();
}

void Session::receiveValue(muduo::net::Buffer* buf)
{
  assert(currItem_.get());
  assert(state_ == kReceiveValue);
  // if (protocol_ == kBinary)

  const size_t avail = std::min(buf->readableBytes(), currItem_->neededBytes());
  assert(currItem_.unique());
  currItem_->append(buf->peek(), avail);
  buf->retrieve(avail);
  if (currItem_->neededBytes() == 0)
  {
    if (currItem_->endsWithCRLF())
    {
      bool exists = false;
      if (owner_->storeItem(currItem_, policy_, &exists))
      {
        reply("STORED\r\n");
      }
      else
      {
        if (policy_ == Item::kCas)
        {
          if (exists)
          {
            reply("EXISTS\r\n");
          }
          else
          {
            reply("NOT_FOUND\r\n");
          }
        }
        else
        {
          reply("NOT_STORED\r\n");
        }
      }
    }
    else
    {
      reply("CLIENT_ERROR bad data chunk\r\n");
    }
    resetRequest();
    state_ = kNewCommand;
  }
}

void Session::discardValue(muduo::net::Buffer* buf)
{
  assert(!currItem_);
  assert(state_ == kDiscardValue);
  if (buf->readableBytes() < bytesToDiscard_)
  {
    bytesToDiscard_ -= buf->readableBytes();
    buf->retrieveAll();
  }
  else
  {
    buf->retrieve(bytesToDiscard_);
    bytesToDiscard_ = 0;
    resetRequest();
    state_ = kNewCommand;
  }
}

bool Session::processRequest(StringPiece request)
{
  assert(command_.empty());
  assert(!noreply_);
  assert(policy_ == Item::kInvalid);
  assert(!currItem_);
  assert(bytesToDiscard_ == 0);
  ++requestsProcessed_;

  // check 'noreply' at end of request line
  if (request.size() >= 8)
  {
    StringPiece end(request.end() - 8, 8);
    if (end == " noreply")
    {
      noreply_ = true;
      request.remove_suffix(8);
    }
  }

  SpaceSeparator sep;
  Tokenizer tok(request.begin(), request.end(), sep);
  Tokenizer::iterator beg = tok.begin();
  if (beg == tok.end())
  {
    reply("ERROR\r\n");
    return true;
  }
  (*beg).CopyToString(&command_);
  ++beg;
  if (command_ == "set" || command_ == "add" || command_ == "replace"
      || command_ == "append" || command_ == "prepend" || command_ == "cas")
  {
    // this normally returns false
    return doUpdate(beg, tok.end());
  }
  else if (command_ == "get" || command_ == "gets")
  {
    bool cas = command_ == "gets";

    // FIXME: send multiple chunks with write complete callback.
    while (beg != tok.end())
    {
      StringPiece key = *beg;
      bool good = key.size() <= kLongestKeySize;
      if (!good)
      {
        reply("CLIENT_ERROR bad command line format\r\n");
        return true;
      }

      needle_->resetKey(key);
      ConstItemPtr item = owner_->getItem(needle_);
      ++beg;
      if (item)
      {
        item->output(&outputBuf_, cas);
      }
    }
    outputBuf_.append("END\r\n");

    if (conn_->outputBuffer()->writableBytes() > 65536 + outputBuf_.readableBytes())
    {
      LOG_DEBUG << "shrink output buffer from " << conn_->outputBuffer()->internalCapacity();
      conn_->outputBuffer()->shrink(65536 + outputBuf_.readableBytes());
    }

    conn_->send(&outputBuf_);
  }
  else if (command_ == "delete")
  {
    doDelete(beg, tok.end());
  }
  else if (command_ == "version")
  {
#ifdef HAVE_TCMALLOC
    reply("VERSION 0.01 muduo with tcmalloc\r\n");
#else
    reply("VERSION 0.01 muduo\r\n");
#endif
  }
#ifdef HAVE_TCMALLOC
  else if (command_ == "memstat")
  {
    char buf[1024*64];
    MallocExtension::instance()->GetStats(buf, sizeof buf);
    reply(buf);
  }
#endif
  else if (command_ == "quit")
  {
    conn_->shutdown();
  }
  else if (command_ == "shutdown")
  {
    // "ERROR: shutdown not enabled"
    conn_->shutdown();
    owner_->stop();
  }
  else
  {
    reply("ERROR\r\n");
    LOG_INFO << "Unknown command: " << command_;
  }
  return true;
}

void Session::resetRequest()
{
  command_.clear();
  noreply_ = false;
  policy_ = Item::kInvalid;
  currItem_.reset();
  bytesToDiscard_ = 0;
}

void Session::reply(muduo::StringPiece msg)
{
  if (!noreply_)
  {
    conn_->send(msg.data(), msg.size());
  }
}

bool Session::doUpdate(Session::Tokenizer::iterator& beg, Session::Tokenizer::iterator end)
{
  if (command_ == "set")
    policy_ = Item::kSet;
  else if (command_ == "add")
    policy_ = Item::kAdd;
  else if (command_ == "replace")
    policy_ = Item::kReplace;
  else if (command_ == "append")
    policy_ = Item::kAppend;
  else if (command_ == "prepend")
    policy_ = Item::kPrepend;
  else if (command_ == "cas")
    policy_ = Item::kCas;
  else
    assert(false);

  // FIXME: check (beg != end)
  StringPiece key = (*beg);
  ++beg;
  bool good = key.size() <= kLongestKeySize;

  uint32_t flags = 0;
  time_t exptime = 1;
  int bytes = -1;
  uint64_t cas = 0;

  Reader r(beg, end);
  good = good && r.read(&flags) && r.read(&exptime) && r.read(&bytes);

  int rel_exptime = static_cast<int>(exptime);
  if (exptime > 60*60*24*30)
  {
    rel_exptime = static_cast<int>(exptime - owner_->startTime());
    if (rel_exptime < 1)
    {
      rel_exptime = 1;
    }
  }
  else
  {
    // rel_exptime = exptime + currentTime;
  }

  if (good && policy_ == Item::kCas)
  {
    good = r.read(&cas);
  }

  if (!good)
  {
    reply("CLIENT_ERROR bad command line format\r\n");
    return true;
  }
  if (bytes > 1024*1024)
  {
    reply("SERVER_ERROR object too large for cache\r\n");
    needle_->resetKey(key);
    owner_->deleteItem(needle_);
    bytesToDiscard_ = bytes + 2;
    state_ = kDiscardValue;
    return false;
  }
  else
  {
    currItem_ = Item::makeItem(key, flags, rel_exptime, bytes + 2, cas);
    state_ = kReceiveValue;
    return false;
  }
}

void Session::doDelete(Session::Tokenizer::iterator& beg, Session::Tokenizer::iterator end)
{
  assert(command_ == "delete");
  // FIXME: check (beg != end)
  StringPiece key = *beg;
  bool good = key.size() <= kLongestKeySize;
  ++beg;
  if (!good)
  {
    reply("CLIENT_ERROR bad command line format\r\n");
  }
  else if (beg != end && *beg != "0") // issue 108, old protocol
  {
    reply("CLIENT_ERROR bad command line format.  Usage: delete <key> [noreply]\r\n");
  }
  else
  {
    needle_->resetKey(key);
    if (owner_->deleteItem(needle_))
    {
      reply("DELETED\r\n");
    }
    else
    {
      reply("NOT_FOUND\r\n");
    }
  }
}
