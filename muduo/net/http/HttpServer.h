// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is a public header file, it must only include public header files.

#ifndef MUDUO_NET_HTTP_HTTPSERVER_H
#define MUDUO_NET_HTTP_HTTPSERVER_H

#include <muduo/net/TcpServer.h>

#include <boost/noncopyable.hpp>

#include <map>

namespace muduo
{
namespace net
{

class HttpRequest;
class HttpResponse;

class HttpServer : boost::noncopyable
{
 public:
  typedef boost::function<void (const TcpConnectionPtr&,
                                const HttpRequest&,
                                HttpResponse*)> HttpCallback;

  HttpServer(EventLoop* loop, 
             const InetAddress& listenAddr,
             const string& name);

  ~HttpServer();  // force out-line dtor, for scoped_ptr members.

  void start();
  void registerHttpCallback(const string& path,
                            const HttpCallback& cb);

 private:
  void onConnection(const TcpConnectionPtr& conn);
  void onMessage(const TcpConnectionPtr& conn,
                 Buffer* buf,
                 Timestamp receiveTime);
  void onRequest(const TcpConnectionPtr&, const HttpRequest&);

  TcpServer server_;
  std::map<string, HttpCallback> callbacks_;
};

}
}

#endif  // MUDUO_NET_HTTP_HTTPSERVER_H
