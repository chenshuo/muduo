// Muduo - A lightwight C++ network library for Linux
// Copyright (c) 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Muduo team nor the names of its contributors
// may be used to endorse or promote products derived from this software
// without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is a public header file, it must only include public header files.

#ifndef MUDUO_NET_CHANNELBUFFER_H
#define MUDUO_NET_CHANNELBUFFER_H

#include <muduo/base/copyable.h>
#include <muduo/base/Types.h>

#include <vector>

#include <assert.h>
//#include <unistd.h>  // ssize_t

namespace muduo
{
namespace net
{

/// A buffer class modeled after org.jboss.netty.buffer.ChannelBuffer
///
/// @code
/// +-------------------+------------------+------------------+
/// | prependable bytes |  readable bytes  |  writable bytes  |
/// |                   |     (CONTENT)    |                  |
/// +-------------------+------------------+------------------+
/// |                   |                  |                  |
/// 0      <=      readerIndex   <=   writerIndex    <=     size
/// @endcode
class ChannelBuffer : public muduo::copyable
{
 public:
  static const size_t kCheapPrepend = 8;
  static const size_t kInitialSize = 1024;

  ChannelBuffer()
    : buffer_(kCheapPrepend + kInitialSize),
      readerIndex_(kCheapPrepend),
      writerIndex_(kCheapPrepend)
  {
    assert(readableBytes() == 0);
    assert(writableBytes() == kInitialSize);
    assert(prependableBytes() == kCheapPrepend);
  }

  // default copy-ctor, dtor and assignment are fine

  void swap(ChannelBuffer& rhs)
  {
    buffer_.swap(rhs.buffer_);
    std::swap(readerIndex_, rhs.readerIndex_);
    std::swap(writerIndex_, rhs.writerIndex_);
  }

  size_t readableBytes()
  { return writerIndex_ - readerIndex_; }

  size_t writableBytes()
  { return buffer_.size() - writerIndex_; }

  size_t prependableBytes()
  { return readerIndex_; }

  const char* peek() const
  { return begin() + readerIndex_; }

  // retrieve returns void, to prevent
  // string str(retrieve(readableBytes()), readableBytes());
  // the evaluation of two functions are unspecified
  void retrieve(size_t len)
  {
    assert(len <= readableBytes());
    readerIndex_ += len;
  }

  void retrieveAll()
  {
    readerIndex_ = kCheapPrepend;
    writerIndex_ = kCheapPrepend;
  }

  string retrieveAsString()
  {
    string str(peek(), readableBytes());
    readerIndex_ = kCheapPrepend;
    writerIndex_ = kCheapPrepend;
    return str;
  }

  void append(const char* /*restrict*/ data, size_t len)
  {
    if (writableBytes() < len)
    {
      makeSpace(len);
    }
    assert(len <= writableBytes());
    std::copy(data, data+len, begin()+writerIndex_);
    writerIndex_ += len;
  }

  void prepend(const char* /*restrict*/ data, size_t len)
  {
    assert(len <= prependableBytes());
    readerIndex_ -= len;
    std::copy(data, data+len, begin()+readerIndex_);
  }

  void shrink(size_t reserve)
  {
   std::vector<char> buf(kCheapPrepend+readableBytes()+reserve);
   std::copy(peek(), peek()+readableBytes(), buf.begin()+kCheapPrepend);
   buf.swap(buffer_);
  }

  /// Read data directly into buffer.
  ///
  /// It may implement with readv(2)
  /// @return result of read(2), @c errno is saved
  ssize_t readFd(int fd, int* savedErrno);

 private:
   char* begin()
   { return &*buffer_.begin(); }

   const char* begin() const
   { return &*buffer_.begin(); }

   void makeSpace(size_t more)
   {
     if (writableBytes() + prependableBytes() < more + kCheapPrepend)
     {
       buffer_.resize(writerIndex_+more);
     }
     else
     {
       // move readable data to the front, make space inside buffer
       size_t used = readableBytes();
       std::copy(begin()+readerIndex_,
                 begin()+writerIndex_,
                 begin()+kCheapPrepend);
       readerIndex_ = kCheapPrepend;
       writerIndex_ = readerIndex_ + used;
       assert(used = readableBytes());
     }
   }

 private:
   std::vector<char> buffer_;
   size_t readerIndex_;
   size_t writerIndex_;
};

}
}

#endif  // MUDUO_NET_CHANNELBUFFER_H
