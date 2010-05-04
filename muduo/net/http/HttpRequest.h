// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is a public header file, it must only include public header files.

#ifndef MUDUO_NET_HTTP_HTTPREQUEST_H
#define MUDUO_NET_HTTP_HTTPREQUEST_H

#include <boost/noncopyable.hpp>

#include <map>

namespace muduo
{
namespace net
{

class HttpRequest
{
 public:
  enum Method
  {
    kUnknown, kGet, kPost, kHead, kPut, kDelete
  };

  HttpRequest()
    : method_(kUnknown)
  {
  }

  bool setMethod(const char* start, const char* end)
  {
    assert(method_ == kUnknown);
    string method(start, end);
    if (method == "GET")
    {
      method_ = kGet;
    }
    else if (method == "POST")
    {
      method_ = kPost;
    }
    else if (method == "HEAD")
    {
      method_ = kHead;
    }
    else if (method == "PUT")
    {
      method_ = kPut;
    }
    else if (method == "DELETE")
    {
      method_ = kDelete;
    }
    else
    {
      method_ = kUnknown;
    }
    return method_ != kUnknown;
  }

  void setUri(const char* start, const char* end)
  {
    uri_.assign(start, end);
  }

  const string& uri() const
  { return uri_; }

  void setReceiveTime(Timestamp t)
  { receiveTime_ = t; }

  void addHeader(const char* start, const char* colon, const char* end)
  {
    string field(start, colon);
    ++colon;
    while (isspace(*colon))
    {
      ++colon;
    }
    string value(colon, end);
    while (!value.empty() && isspace(value[value.size()-1]))
    {
      value.resize(value.size()-1);
    }
    headers_[field] = value;
  }

  const std::map<string, string>& headers() const
  { return headers_; }

 private:
  Method method_;
  string uri_;
  Timestamp receiveTime_;
  std::map<string, string> headers_;
};

}
}

#endif  // MUDUO_NET_HTTP_HTTPREQUEST_H
