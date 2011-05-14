// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//

#include <muduo/net/Buffer.h>
#include <muduo/net/SocketsOps.h>

#include <errno.h>
#include <memory.h>
#include <sys/uio.h>

using namespace muduo;
using namespace muduo::net;

const char Buffer::kCRLF[] = "\r\n";

void Buffer::appendInt32(int32_t x)
{
  int32_t be32 = sockets::hostToNetwork32(x);
  append(&be32, sizeof be32);
}

int32_t Buffer::peekInt32() const
{
  assert(readableBytes() >= sizeof(int32_t));
  int32_t be32 = 0;
  ::memcpy(&be32, peek(), sizeof be32);
  return sockets::networkToHost32(be32);
}

ssize_t Buffer::readFd(int fd, int* savedErrno)
{
  // FIXME use ioctl/FIONREAD to tell how much to read
  char extrabuf[65536];
  struct iovec vec[2];
  size_t writable = writableBytes();
  vec[0].iov_base = begin()+writerIndex_;
  vec[0].iov_len = writable;
  vec[1].iov_base = extrabuf;
  vec[1].iov_len = sizeof extrabuf;
  ssize_t n = sockets::readv(fd, vec, 2);
  if (n < 0)
  {
    *savedErrno = errno;
  }
  else if (implicit_cast<size_t>(n) <= writable)
  {
    writerIndex_ += n;
  }
  else
  {
    writerIndex_ = buffer_.size();
    append(extrabuf, n - writable);
  }
  return n;
}

