// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//

#include <muduo/net/inspect/Inspector.h>

#include <muduo/base/Thread.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/http/HttpRequest.h>
#include <muduo/net/http/HttpResponse.h>
#include <muduo/net/inspect/ProcessInspector.h>

//#include <iostream>
//#include <iterator>
//#include <sstream>
#include <boost/bind.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

using namespace muduo;
using namespace muduo::net;

namespace
{
Inspector* g_globalInspector = 0;

// Looks buggy
std::vector<string> split(const string& str)
{
  std::vector<string> result;
  size_t start = 0;
  size_t pos = str.find('/');
  while (pos != string::npos)
  {
    if (pos > start)
    {
      result.push_back(str.substr(start, pos-start));
    }
    start = pos+1;
    pos = str.find('/', start);
  }

  if (start < str.length())
  {
    result.push_back(str.substr(start));
  }

  return result;
}

}

Inspector::Inspector(EventLoop* loop,
                     const InetAddress& httpAddr,
                     const string& name)
    : server_(loop, httpAddr, "Inspector:"+name),
      processInspector_(new ProcessInspector)
{
  assert(CurrentThread::isMainThread());
  assert(g_globalInspector == 0);
  g_globalInspector = this;
  server_.setHttpCallback(boost::bind(&Inspector::onRequest, this, _1, _2));
  processInspector_->registerCommands(this);
  loop->runAfter(0, boost::bind(&Inspector::start, this)); // little race condition
}

Inspector::~Inspector()
{
  assert(CurrentThread::isMainThread());
  g_globalInspector = NULL;
}

void Inspector::add(const string& module,
                    const string& command,
                    const Callback& cb,
                    const string& help)
{
  MutexLockGuard lock(mutex_);
  commands_[module][command] = cb;
  helps_[module][command] = help;
}

void Inspector::start()
{
  server_.start();
}

void Inspector::onRequest(const HttpRequest& req, HttpResponse* resp)
{
  if (req.path() == "/")
  {
    string result;
    MutexLockGuard lock(mutex_);
    for (std::map<string, HelpList>::const_iterator helpListI = helps_.begin();
         helpListI != helps_.end();
         ++helpListI)
    {
      const HelpList& list = helpListI->second;
      for (HelpList::const_iterator it = list.begin();
           it != list.end();
           ++it)
      {
        result += "/";
        result += helpListI->first;
        result += "/";
        result += it->first;
        result += "\t";
        result += it->second;
        result += "\n";
      }
    }
    resp->setStatusCode(HttpResponse::k200Ok);
    resp->setStatusMessage("OK");
    resp->setContentType("text/plain");
    resp->setBody(result);
  }
  else
  {
    std::vector<string> result = split(req.path());
    // boost::split(result, req.path(), boost::is_any_of("/"));
    //std::copy(result.begin(), result.end(), std::ostream_iterator<string>(std::cout, ", "));
    //std::cout << "\n";
    bool ok = false;
    if (result.size() == 0)
    {
    }
    else if (result.size() == 1)
    {
      string module = result[0];
    }
    else
    {
      string module = result[0];
      std::map<string, CommandList>::const_iterator commListI = commands_.find(module);
      if (commListI != commands_.end())
      {
        string command = result[1];
        const CommandList& commList = commListI->second;
        CommandList::const_iterator it = commList.find(command);
        if (it != commList.end())
        {
          ArgList args(result.begin()+2, result.end());
          if (it->second)
          {
            resp->setStatusCode(HttpResponse::k200Ok);
            resp->setStatusMessage("OK");
            resp->setContentType("text/plain");
            const Callback& cb = it->second;
            resp->setBody(cb(req.method(), args));
            ok = true;
          }
        }
      }

    }

    if (!ok)
    {
      resp->setStatusCode(HttpResponse::k404NotFound);
      resp->setStatusMessage("Not Found");
    }
    //resp->setCloseConnection(true);
  }
}

