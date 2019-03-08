#include "examples/memcached/server/Item.h"

#include "muduo/base/LogStream.h"
#include "muduo/net/Buffer.h"

#include <boost/functional/hash/hash.hpp>

#include <string.h> // memcpy
#include <stdio.h>

using namespace muduo;
using namespace muduo::net;

Item::Item(StringPiece keyArg,
           uint32_t flagsArg,
           int exptimeArg,
           int valuelen,
           uint64_t casArg)
  : keylen_(keyArg.size()),
    flags_(flagsArg),
    rel_exptime_(exptimeArg),
    valuelen_(valuelen),
    receivedBytes_(0),
    cas_(casArg),
    hash_(boost::hash_range(keyArg.begin(), keyArg.end())),
    data_(static_cast<char*>(::malloc(totalLen())))
{
  assert(valuelen_ >= 2);
  assert(receivedBytes_ < totalLen());
  append(keyArg.data(), keylen_);
}

void Item::append(const char* data, size_t len)
{
  assert(len <= neededBytes());
  memcpy(data_ + receivedBytes_, data, len);
  receivedBytes_ += static_cast<int>(len);
  assert(receivedBytes_ <= totalLen());
}

void Item::output(Buffer* out, bool needCas) const
{
  out->append("VALUE ");
  out->append(data_, keylen_);
  LogStream buf;
  buf << ' ' << flags_ << ' ' << valuelen_-2;
  if (needCas)
  {
    buf << ' ' << cas_;
  }
  buf << "\r\n";
  out->append(buf.buffer().data(), buf.buffer().length());
  out->append(value(), valuelen_);
}

void Item::resetKey(StringPiece k)
{
  assert(k.size() <= 250);
  keylen_ = k.size();
  receivedBytes_ = 0;
  append(k.data(), k.size());
  hash_ = boost::hash_range(k.begin(), k.end());
}
