// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//

#include <muduo/base/FileUtil.h>

#include <boost/static_assert.hpp>

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>

using namespace muduo;

template<typename String>
int FileUtil::readFile(StringPiece filename,
                       size_t maxSize,
                       String* content,
                       int64_t* fileSize)
{
  BOOST_STATIC_ASSERT(sizeof(off_t) == 8);
  assert(content != NULL);
  content->clear();
  int err = 0;

  FILE* fp = fopen(filename.data(), "r");
  if (fp)
  {
    // FIXME: ::setbuffer()
    while (!feof(fp) && content->size() < maxSize)
    {
      char buf[8192];
      size_t n = fread(buf, 1, sizeof buf, fp);
      if (n == 0)
      {
        err = ferror(fp);
        break;
      }
      if (content->size() + n > maxSize) // FIXME: interger overflow
      {
        n = maxSize - content->size();
      }
      content->append(buf, n);
    }
    if (fileSize)
    {
      struct stat statbuf;
      if (fstat(fileno(fp), &statbuf) == 0)
      {
        if (S_ISREG(statbuf.st_mode))
        {
          *fileSize = statbuf.st_size;
        }
        else if (S_ISDIR(statbuf.st_mode))
        {
          err = EISDIR;
        }
      }
      else
      {
        err = errno;
      }
    }
    fclose(fp);
  }
  else
  {
    err = errno;
  }
  return err;
}

template int FileUtil::readFile(StringPiece filename,
                                size_t maxSize,
                                string* content,
                                int64_t* fileSize);

#ifndef MUDUO_STD_STRING
template int FileUtil::readFile(StringPiece filename,
                                size_t maxSize,
                                std::string* content,
                                int64_t* fileSize);
#endif

