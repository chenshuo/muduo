#include "Item.h"

#include <muduo/net/Buffer.h>

#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <string.h> // memcpy
#include <stdio.h>

using namespace muduo::net;

void Item::append(const char* data, size_t len)
{
  assert(len <= neededBytes());
  memcpy(value_ + receivedBytes_, data, len);
  receivedBytes_ += static_cast<int>(len);
  assert(receivedBytes_ <= valuelen_);
}

void Item::output(Buffer* out, bool needCas) const
{
  out->append("VALUE ");
  out->append(key_);
  char buf[64];
  if (needCas)
  {
    snprintf(buf, sizeof buf, " %u %d %" PRIu64 "\r\n", flags_, valuelen_-2, cas_);
  }
  else
  {
    snprintf(buf, sizeof buf, " %u %d\r\n", flags_, valuelen_-2);
  }
  out->append(buf);
  out->append(value_, valuelen_);
}

