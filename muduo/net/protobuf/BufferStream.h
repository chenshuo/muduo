// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is a public header file, it must only include public header files.
#pragma once
#include "muduo/net/Buffer.h"
#include <google/protobuf/io/zero_copy_stream.h>
namespace muduo
{
namespace net
{

// FIXME:
// class BufferInputStream : google::protobuf::io::ZeroCopyInputStream
// {
// };

class BufferOutputStream : public google::protobuf::io::ZeroCopyOutputStream
{
 public:
  BufferOutputStream(Buffer* buf)
    : buffer_(CHECK_NOTNULL(buf)),
      originalSize_(buffer_->readableBytes())
  {
  }

  virtual bool Next(void** data, int* size) // override
  {
    buffer_->ensureWritableBytes(4096);
    *data = buffer_->beginWrite();
    *size = static_cast<int>(buffer_->writableBytes());
    buffer_->hasWritten(*size);
    return true;
  }

  virtual void BackUp(int count) // override
  {
    buffer_->unwrite(count);
  }

  virtual int64_t ByteCount() const // override
  {
    return buffer_->readableBytes() - originalSize_;
  }

 private:
  Buffer* buffer_;
  size_t originalSize_;
};

}
}
