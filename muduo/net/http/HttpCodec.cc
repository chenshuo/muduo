// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//

#include <muduo/net/http/HttpCodec.h>
#include <muduo/net/http/HttpContext.h>

using namespace muduo;
using namespace muduo::net;

namespace
{
  bool processRequestLine(const char* begin, const char* end, HttpContext* context)
  {
    bool succeed = false;
    const char* start = begin;
    const char* space = std::find(start, end, ' ');
    if (space != end && context->request().setMethod(start, space))
    {
      start = space+1;
      space = std::find(start, end, ' ');
      if (space != end)
      {
        context->request().setUri(start, space);
        start = space+1;
        succeed = end-start == 8 && std::equal(start, end, "HTTP/1.1");
      }
    }
    return succeed;
  }

}

void HttpCodec::onMessage(const TcpConnectionPtr& conn,
                          Buffer* buf,
                          Timestamp receiveTime)
{
  HttpContext& context = boost::any_cast<HttpContext&>(conn->getContext());
  bool hasMore = true;
  while (hasMore)
  {
    if (context.expectRequestLine())
    {
      const char* crlf = buf->findCRLF();
      if (crlf)
      {
        if (processRequestLine(buf->peek(), crlf, &context))
        {
          buf->retrieve(crlf - buf->peek() + 2);
          context.receiveRequestLine();
        }
        else
        {
          conn->shutdown();
        }
      }
      else
      {
        hasMore = false;
      }
    }
    else if (context.expectHeaders())
    {
      const char* crlf = buf->findCRLF();
      if (crlf)
      {
        const char* colon = std::find(buf->peek(), crlf, ':');
        if (colon != crlf)
        {
          context.request().addHeader(buf->peek(), colon, crlf);
        }
        else
        {
          context.receiveHeaders();
          hasMore = !context.gotAll();
        }
        buf->retrieve(crlf - buf->peek() + 2);
      }
      else
      {
        hasMore = false;
      }
    }
    else if (context.expectBody())
    {
    }
  }

  if (context.gotAll())
  {
    requestCallback_(conn, context.request());
    context.reset();
  }
}

