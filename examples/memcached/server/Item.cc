#include "Item.h"

#include <muduo/net/Buffer.h>

#include <boost/unordered_map.hpp>

#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <string.h> // memcpy
#include <stdio.h>

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
  char buf[64];
  // FIXME: replace with LogStream
  if (needCas)
  {
    snprintf(buf, sizeof buf, " %u %d %" PRIu64 "\r\n", flags_, valuelen_-2, cas_);
  }
  else
  {
    snprintf(buf, sizeof buf, " %u %d\r\n", flags_, valuelen_-2);
  }
  out->append(buf);
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
