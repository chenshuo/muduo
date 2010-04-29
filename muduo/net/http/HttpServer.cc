// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//

#include <muduo/net/http/HttpServer.h>
#include <muduo/net/http/HttpCodec.h>
#include <muduo/net/http/HttpContext.h>

#include <boost/bind.hpp>
#include <iostream>

using namespace muduo;
using namespace muduo::net;

namespace
{
}

HttpServer::HttpServer(EventLoop* loop, 
                       const InetAddress& listenAddr,
                       const string& name)
  : server_(loop, listenAddr, name),
    codec_(new HttpCodec(boost::bind(&HttpServer::onRequest, this, _1, _2)))
{
  server_.setConnectionCallback(
      boost::bind(&HttpServer::onConnection, this, _1));
  server_.setMessageCallback(
      boost::bind(&HttpCodec::onMessage, get_pointer(codec_), _1, _2, _3));
}

HttpServer::~HttpServer()
{
}

void HttpServer::start()
{
  server_.start();
}

void HttpServer::onConnection(const TcpConnectionPtr& conn)
{
  if (conn->connected())
  {
    conn->setContext(HttpContext());
  }
}

void HttpServer::onRequest(const TcpConnectionPtr& conn, const HttpRequest& req)
{
  std::cout << "Headers " << req.uri() << std::endl;
  const std::map<string, string>& headers = req.headers();
  for (std::map<string, string>::const_iterator it = headers.begin();
       it != headers.end();
       ++it)
  {
    std::cout << it->first << ": " << it->second << std::endl;
  }

  conn->send("HTTP/1.1 200 OK\r\n");
  conn->send("Connection: close\r\n");
  conn->send("\r\n");
  conn->send("Hello world.\r\n");
  conn->shutdown();
}

