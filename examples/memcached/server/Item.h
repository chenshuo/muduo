#ifndef MUDUO_EXAMPLES_MEMCACHED_SERVER_ITEM_H
#define MUDUO_EXAMPLES_MEMCACHED_SERVER_ITEM_H

#include "muduo/base/Atomic.h"
#include "muduo/base/StringPiece.h"
#include "muduo/base/Types.h"

#include <memory>

namespace muduo
{
namespace net
{
class Buffer;
}
}

class Item;
typedef std::shared_ptr<Item> ItemPtr;  // TODO: use unique_ptr
typedef std::shared_ptr<const Item> ConstItemPtr;  // TODO: use unique_ptr

// Item is immutable once added into hash table
class Item : muduo::noncopyable
{
 public:
  enum UpdatePolicy
  {
    kInvalid,
    kSet,
    kAdd,
    kReplace,
    kAppend,
    kPrepend,
    kCas,
  };

  static ItemPtr makeItem(muduo::StringPiece keyArg,
                          uint32_t flagsArg,
                          int exptimeArg,
                          int valuelen,
                          uint64_t casArg)
  {
    return std::make_shared<Item>(keyArg, flagsArg, exptimeArg, valuelen, casArg);
    //return ItemPtr(new Item(keyArg, flagsArg, exptimeArg, valuelen, casArg));
  }

  Item(muduo::StringPiece keyArg,
       uint32_t flagsArg,
       int exptimeArg,
       int valuelen,
       uint64_t casArg);

  ~Item()
  {
    ::free(data_);
  }

  muduo::StringPiece key() const
  {
    return muduo::StringPiece(data_, keylen_);
  }

  uint32_t flags() const
  {
    return flags_;
  }

  int rel_exptime() const
  {
    return rel_exptime_;
  }

  const char* value() const
  {
    return data_+keylen_;
  }

  size_t valueLength() const
  {
    return valuelen_;
  }

  uint64_t cas() const
  {
    return cas_;
  }

  size_t hash() const
  {
    return hash_;
  }

  void setCas(uint64_t casArg)
  {
    cas_ = casArg;
  }

  size_t neededBytes() const
  {
    return totalLen() - receivedBytes_;
  }

  void append(const char* data, size_t len);

  bool endsWithCRLF() const
  {
    return receivedBytes_ == totalLen()
        && data_[totalLen()-2] == '\r'
        && data_[totalLen()-1] == '\n';
  }

  void output(muduo::net::Buffer* out, bool needCas = false) const;

  void resetKey(muduo::StringPiece k);

 private:
  int totalLen() const { return keylen_ + valuelen_; }

  int            keylen_;
  const uint32_t flags_;
  const int      rel_exptime_;
  const int      valuelen_;
  int            receivedBytes_;  // FIXME: remove this member
  uint64_t       cas_;
  size_t         hash_;
  char*          data_;
};

#endif  // MUDUO_EXAMPLES_MEMCACHED_SERVER_ITEM_H
