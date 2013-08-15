#ifndef MUDUO_EXAMPLES_MEMCACHED_SERVER_ITEM_H
#define MUDUO_EXAMPLES_MEMCACHED_SERVER_ITEM_H

#include <muduo/base/Atomic.h>
#include <muduo/base/Types.h>

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

using muduo::string;

namespace muduo
{
namespace net
{
class Buffer;
}
}

// Item is immutable once added into hash table
class Item : boost::noncopyable
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

  // TODO: make_shared()
  Item(const string& keyArg, uint32_t flagsArg, time_t exptimeArg, int valuelen, uint64_t casArg)
    : key_(keyArg),
      flags_(flagsArg),
      exptime_(exptimeArg),
      valuelen_(valuelen),
      receivedBytes_(0),
      cas_(casArg),
      value_(static_cast<char*>(::malloc(valuelen)))
  {
    assert(valuelen_ >= 2);
    assert(receivedBytes_ < valuelen_);
  }

  ~Item()
  {
    ::free(value_);
  }

  const string& key() const
  {
    return key_;
  }

  uint32_t flags() const
  {
    return flags_;
  }

  time_t exptime() const
  {
    return exptime_;
  }

  const char* value() const
  {
    return value_;
  }

  size_t valueLength() const
  {
    return valuelen_;
  }

  uint64_t cas() const
  {
    return cas_;
  }

  void setCas(uint64_t casArg)
  {
    cas_ = casArg;
  }

  size_t neededBytes() const
  {
    return valuelen_ - receivedBytes_;
  }

  void append(const char* data, size_t len);

  bool endsWithCRLF() const
  {
    return receivedBytes_ == valuelen_
        && value_[valuelen_-2] == '\r'
        && value_[valuelen_-1] == '\n';
  }

  void output(muduo::net::Buffer* out, bool needCas = false) const;

 private:
  const string key_;
  uint32_t flags_;
  time_t   exptime_;
  int      valuelen_;
  int      receivedBytes_;
  uint64_t cas_;
  char*    value_;

};
typedef boost::shared_ptr<Item> ItemPtr;  // TODO: use unique_ptr
typedef boost::shared_ptr<const Item> ConstItemPtr;  // TODO: use unique_ptr

#endif  // MUDUO_EXAMPLES_MEMCACHED_SERVER_ITEM_H
