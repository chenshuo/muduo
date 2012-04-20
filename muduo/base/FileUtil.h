// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is a public header file, it must only include public header files.

#ifndef MUDUO_BASE_FILEUTIL_H
#define MUDUO_BASE_FILEUTIL_H

#include <muduo/base/Types.h>
#include <muduo/base/StringPiece.h>

namespace muduo
{

namespace FileUtil
{
  // read the file content, returns errno if error happens.
  template<typename String>
  int readFile(StringPiece filename,
               size_t maxSize,
               String* content,
               int64_t* fileSize);

}

}

#endif  // MUDUO_BASE_FILEUTIL_H

