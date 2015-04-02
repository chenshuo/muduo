// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//

#include <muduo/net/inspect/Inspector.h>

#include <muduo/base/Logging.h>
#include <muduo/base/Thread.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/http/HttpRequest.h>
#include <muduo/net/http/HttpResponse.h>
#include <muduo/net/inspect/ProcessInspector.h>
#include <muduo/net/inspect/PerformanceInspector.h>
#include <muduo/net/inspect/SystemInspector.h>

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

extern char favicon[1743];

Inspector::Inspector(EventLoop* loop,
                     const InetAddress& httpAddr,
                     const string& name)
    : server_(loop, httpAddr, "Inspector:"+name),
      processInspector_(new ProcessInspector),
      systemInspector_(new SystemInspector)
{
  assert(CurrentThread::isMainThread());
  assert(g_globalInspector == 0);
  g_globalInspector = this;
  server_.setHttpCallback(boost::bind(&Inspector::onRequest, this, _1, _2));
  processInspector_->registerCommands(this);
  systemInspector_->registerCommands(this);
#ifdef HAVE_TCMALLOC
  performanceInspector_.reset(new PerformanceInspector);
  performanceInspector_->registerCommands(this);
#endif
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
  modules_[module][command] = cb;
  helps_[module][command] = help;
}

void Inspector::remove(const string& module, const string& command)
{
  MutexLockGuard lock(mutex_);
  std::map<string, CommandList>::iterator it = modules_.find(module);
  if (it != modules_.end())
  {
    it->second.erase(command);
    helps_[module].erase(command);
  }
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
        size_t len = helpListI->first.size() + it->first.size();
        result += string(len >= 25 ? 1 : 25 - len, ' ');
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
      LOG_DEBUG << req.path();
    }
    else if (result.size() == 1)
    {
      string module = result[0];
      if (module == "favicon.ico")
      {
        resp->setStatusCode(HttpResponse::k200Ok);
        resp->setStatusMessage("OK");
        resp->setContentType("image/png");
        resp->setBody(string(favicon, sizeof favicon));

        ok = true;
      }
      else
      {
        LOG_ERROR << "Unimplemented " << module;
      }
    }
    else
    {
      string module = result[0];
      std::map<string, CommandList>::const_iterator commListI = modules_.find(module);
      if (commListI != modules_.end())
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

char favicon[1743] =
{
  '\x89', '\x50', '\x4e', '\x47', '\x0d', '\x0a', '\x1a', '\x0a', '\x00', '\x00',
  '\x00', '\x0d', '\x49', '\x48', '\x44', '\x52', '\x00', '\x00', '\x00', '\x10',
  '\x00', '\x00', '\x00', '\x10', '\x08', '\x06', '\x00', '\x00', '\x00', '\x1f',
  '\xf3', '\xff', '\x61', '\x00', '\x00', '\x00', '\x04', '\x73', '\x42', '\x49',
  '\x54', '\x08', '\x08', '\x08', '\x08', '\x7c', '\x08', '\x64', '\x88', '\x00',
  '\x00', '\x00', '\x09', '\x70', '\x48', '\x59', '\x73', '\x00', '\x00', '\x0b',
  '\x12', '\x00', '\x00', '\x0b', '\x12', '\x01', '\xd2', '\xdd', '\x7e', '\xfc',
  '\x00', '\x00', '\x00', '\x1c', '\x74', '\x45', '\x58', '\x74', '\x53', '\x6f',
  '\x66', '\x74', '\x77', '\x61', '\x72', '\x65', '\x00', '\x41', '\x64', '\x6f',
  '\x62', '\x65', '\x20', '\x46', '\x69', '\x72', '\x65', '\x77', '\x6f', '\x72',
  '\x6b', '\x73', '\x20', '\x43', '\x53', '\x33', '\x98', '\xd6', '\x46', '\x03',
  '\x00', '\x00', '\x00', '\x15', '\x74', '\x45', '\x58', '\x74', '\x43', '\x72',
  '\x65', '\x61', '\x74', '\x69', '\x6f', '\x6e', '\x20', '\x54', '\x69', '\x6d',
  '\x65', '\x00', '\x32', '\x2f', '\x31', '\x37', '\x2f', '\x30', '\x38', '\x20',
  '\x9c', '\xaa', '\x58', '\x00', '\x00', '\x04', '\x11', '\x74', '\x45', '\x58',
  '\x74', '\x58', '\x4d', '\x4c', '\x3a', '\x63', '\x6f', '\x6d', '\x2e', '\x61',
  '\x64', '\x6f', '\x62', '\x65', '\x2e', '\x78', '\x6d', '\x70', '\x00', '\x3c',
  '\x3f', '\x78', '\x70', '\x61', '\x63', '\x6b', '\x65', '\x74', '\x20', '\x62',
  '\x65', '\x67', '\x69', '\x6e', '\x3d', '\x22', '\x20', '\x20', '\x20', '\x22',
  '\x20', '\x69', '\x64', '\x3d', '\x22', '\x57', '\x35', '\x4d', '\x30', '\x4d',
  '\x70', '\x43', '\x65', '\x68', '\x69', '\x48', '\x7a', '\x72', '\x65', '\x53',
  '\x7a', '\x4e', '\x54', '\x63', '\x7a', '\x6b', '\x63', '\x39', '\x64', '\x22',
  '\x3f', '\x3e', '\x0a', '\x3c', '\x78', '\x3a', '\x78', '\x6d', '\x70', '\x6d',
  '\x65', '\x74', '\x61', '\x20', '\x78', '\x6d', '\x6c', '\x6e', '\x73', '\x3a',
  '\x78', '\x3d', '\x22', '\x61', '\x64', '\x6f', '\x62', '\x65', '\x3a', '\x6e',
  '\x73', '\x3a', '\x6d', '\x65', '\x74', '\x61', '\x2f', '\x22', '\x20', '\x78',
  '\x3a', '\x78', '\x6d', '\x70', '\x74', '\x6b', '\x3d', '\x22', '\x41', '\x64',
  '\x6f', '\x62', '\x65', '\x20', '\x58', '\x4d', '\x50', '\x20', '\x43', '\x6f',
  '\x72', '\x65', '\x20', '\x34', '\x2e', '\x31', '\x2d', '\x63', '\x30', '\x33',
  '\x34', '\x20', '\x34', '\x36', '\x2e', '\x32', '\x37', '\x32', '\x39', '\x37',
  '\x36', '\x2c', '\x20', '\x53', '\x61', '\x74', '\x20', '\x4a', '\x61', '\x6e',
  '\x20', '\x32', '\x37', '\x20', '\x32', '\x30', '\x30', '\x37', '\x20', '\x32',
  '\x32', '\x3a', '\x31', '\x31', '\x3a', '\x34', '\x31', '\x20', '\x20', '\x20',
  '\x20', '\x20', '\x20', '\x20', '\x20', '\x22', '\x3e', '\x0a', '\x20', '\x20',
  '\x20', '\x3c', '\x72', '\x64', '\x66', '\x3a', '\x52', '\x44', '\x46', '\x20',
  '\x78', '\x6d', '\x6c', '\x6e', '\x73', '\x3a', '\x72', '\x64', '\x66', '\x3d',
  '\x22', '\x68', '\x74', '\x74', '\x70', '\x3a', '\x2f', '\x2f', '\x77', '\x77',
  '\x77', '\x2e', '\x77', '\x33', '\x2e', '\x6f', '\x72', '\x67', '\x2f', '\x31',
  '\x39', '\x39', '\x39', '\x2f', '\x30', '\x32', '\x2f', '\x32', '\x32', '\x2d',
  '\x72', '\x64', '\x66', '\x2d', '\x73', '\x79', '\x6e', '\x74', '\x61', '\x78',
  '\x2d', '\x6e', '\x73', '\x23', '\x22', '\x3e', '\x0a', '\x20', '\x20', '\x20',
  '\x20', '\x20', '\x20', '\x3c', '\x72', '\x64', '\x66', '\x3a', '\x44', '\x65',
  '\x73', '\x63', '\x72', '\x69', '\x70', '\x74', '\x69', '\x6f', '\x6e', '\x20',
  '\x72', '\x64', '\x66', '\x3a', '\x61', '\x62', '\x6f', '\x75', '\x74', '\x3d',
  '\x22', '\x22', '\x0a', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20',
  '\x20', '\x20', '\x20', '\x20', '\x20', '\x78', '\x6d', '\x6c', '\x6e', '\x73',
  '\x3a', '\x78', '\x61', '\x70', '\x3d', '\x22', '\x68', '\x74', '\x74', '\x70',
  '\x3a', '\x2f', '\x2f', '\x6e', '\x73', '\x2e', '\x61', '\x64', '\x6f', '\x62',
  '\x65', '\x2e', '\x63', '\x6f', '\x6d', '\x2f', '\x78', '\x61', '\x70', '\x2f',
  '\x31', '\x2e', '\x30', '\x2f', '\x22', '\x3e', '\x0a', '\x20', '\x20', '\x20',
  '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x3c', '\x78', '\x61', '\x70',
  '\x3a', '\x43', '\x72', '\x65', '\x61', '\x74', '\x6f', '\x72', '\x54', '\x6f',
  '\x6f', '\x6c', '\x3e', '\x41', '\x64', '\x6f', '\x62', '\x65', '\x20', '\x46',
  '\x69', '\x72', '\x65', '\x77', '\x6f', '\x72', '\x6b', '\x73', '\x20', '\x43',
  '\x53', '\x33', '\x3c', '\x2f', '\x78', '\x61', '\x70', '\x3a', '\x43', '\x72',
  '\x65', '\x61', '\x74', '\x6f', '\x72', '\x54', '\x6f', '\x6f', '\x6c', '\x3e',
  '\x0a', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20',
  '\x3c', '\x78', '\x61', '\x70', '\x3a', '\x43', '\x72', '\x65', '\x61', '\x74',
  '\x65', '\x44', '\x61', '\x74', '\x65', '\x3e', '\x32', '\x30', '\x30', '\x38',
  '\x2d', '\x30', '\x32', '\x2d', '\x31', '\x37', '\x54', '\x30', '\x32', '\x3a',
  '\x33', '\x36', '\x3a', '\x34', '\x35', '\x5a', '\x3c', '\x2f', '\x78', '\x61',
  '\x70', '\x3a', '\x43', '\x72', '\x65', '\x61', '\x74', '\x65', '\x44', '\x61',
  '\x74', '\x65', '\x3e', '\x0a', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20',
  '\x20', '\x20', '\x20', '\x3c', '\x78', '\x61', '\x70', '\x3a', '\x4d', '\x6f',
  '\x64', '\x69', '\x66', '\x79', '\x44', '\x61', '\x74', '\x65', '\x3e', '\x32',
  '\x30', '\x30', '\x38', '\x2d', '\x30', '\x33', '\x2d', '\x32', '\x34', '\x54',
  '\x31', '\x39', '\x3a', '\x30', '\x30', '\x3a', '\x34', '\x32', '\x5a', '\x3c',
  '\x2f', '\x78', '\x61', '\x70', '\x3a', '\x4d', '\x6f', '\x64', '\x69', '\x66',
  '\x79', '\x44', '\x61', '\x74', '\x65', '\x3e', '\x0a', '\x20', '\x20', '\x20',
  '\x20', '\x20', '\x20', '\x3c', '\x2f', '\x72', '\x64', '\x66', '\x3a', '\x44',
  '\x65', '\x73', '\x63', '\x72', '\x69', '\x70', '\x74', '\x69', '\x6f', '\x6e',
  '\x3e', '\x0a', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x3c', '\x72',
  '\x64', '\x66', '\x3a', '\x44', '\x65', '\x73', '\x63', '\x72', '\x69', '\x70',
  '\x74', '\x69', '\x6f', '\x6e', '\x20', '\x72', '\x64', '\x66', '\x3a', '\x61',
  '\x62', '\x6f', '\x75', '\x74', '\x3d', '\x22', '\x22', '\x0a', '\x20', '\x20',
  '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20',
  '\x78', '\x6d', '\x6c', '\x6e', '\x73', '\x3a', '\x64', '\x63', '\x3d', '\x22',
  '\x68', '\x74', '\x74', '\x70', '\x3a', '\x2f', '\x2f', '\x70', '\x75', '\x72',
  '\x6c', '\x2e', '\x6f', '\x72', '\x67', '\x2f', '\x64', '\x63', '\x2f', '\x65',
  '\x6c', '\x65', '\x6d', '\x65', '\x6e', '\x74', '\x73', '\x2f', '\x31', '\x2e',
  '\x31', '\x2f', '\x22', '\x3e', '\x0a', '\x20', '\x20', '\x20', '\x20', '\x20',
  '\x20', '\x20', '\x20', '\x20', '\x3c', '\x64', '\x63', '\x3a', '\x66', '\x6f',
  '\x72', '\x6d', '\x61', '\x74', '\x3e', '\x69', '\x6d', '\x61', '\x67', '\x65',
  '\x2f', '\x70', '\x6e', '\x67', '\x3c', '\x2f', '\x64', '\x63', '\x3a', '\x66',
  '\x6f', '\x72', '\x6d', '\x61', '\x74', '\x3e', '\x0a', '\x20', '\x20', '\x20',
  '\x20', '\x20', '\x20', '\x3c', '\x2f', '\x72', '\x64', '\x66', '\x3a', '\x44',
  '\x65', '\x73', '\x63', '\x72', '\x69', '\x70', '\x74', '\x69', '\x6f', '\x6e',
  '\x3e', '\x0a', '\x20', '\x20', '\x20', '\x3c', '\x2f', '\x72', '\x64', '\x66',
  '\x3a', '\x52', '\x44', '\x46', '\x3e', '\x0a', '\x3c', '\x2f', '\x78', '\x3a',
  '\x78', '\x6d', '\x70', '\x6d', '\x65', '\x74', '\x61', '\x3e', '\x0a', '\x20',
  '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20',
  '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20',
  '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20',
  '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20',
  '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20',
  '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20',
  '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20',
  '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20',
  '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20',
  '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x0a',
  '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20',
  '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20',
  '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20',
  '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20',
  '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20',
  '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20',
  '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20',
  '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20',
  '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20',
  '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20',
  '\x0a', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20',
  '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20',
  '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20',
  '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20',
  '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20',
  '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20',
  '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20',
  '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20',
  '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20',
  '\x20', '\x20', '\x35', '\x1d', '\x52', '\x64', '\x00', '\x00', '\x02', '\x0b',
  '\x49', '\x44', '\x41', '\x54', '\x38', '\x8d', '\xa5', '\x93', '\x41', '\x6b',
  '\x1a', '\x41', '\x18', '\x86', '\x9f', '\x89', '\x06', '\x23', '\x2e', '\x74',
  '\xf5', '\x20', '\x4a', '\x0e', '\x6b', '\x20', '\xe4', '\xe2', '\xa5', '\x78',
  '\xc8', '\xa1', '\x78', '\x89', '\x82', '\x17', '\x85', '\x82', '\x82', '\x3d',
  '\x26', '\x62', '\x58', '\x72', '\xcd', '\x4f', '\xf1', '\x18', '\x4b', '\x1b',
  '\x2f', '\xa5', '\x32', '\xe0', '\xb5', '\x9e', '\x35', '\x1a', '\x58', '\x2f',
  '\xe9', '\x0f', '\xe8', '\x65', '\x6d', '\x84', '\xa2', '\x84', '\x2c', '\x0a',
  '\x6b', '\x58', '\x62', '\xa7', '\x87', '\x68', '\x68', '\x13', '\xa1', '\xa5',
  '\x79', '\xe1', '\x3b', '\x0c', '\xc3', '\xfb', '\xcc', '\x37', '\xef', '\x7c',
  '\x23', '\x94', '\x52', '\xbc', '\x44', '\x7e', '\x00', '\x21', '\x04', '\x00',
  '\xad', '\x56', '\xcb', '\x18', '\x8f', '\xc7', '\x0d', '\xdb', '\xb6', '\xd3',
  '\x8e', '\xe3', '\xf8', '\x83', '\xc1', '\xa0', '\x8a', '\xc7', '\xe3', '\x3f',
  '\x12', '\x89', '\x44', '\xb9', '\x5c', '\x2e', '\xf7', '\x56', '\xa6', '\xdf',
  '\x0f', '\x15', '\x4a', '\x29', '\x84', '\x10', '\x34', '\x9b', '\xcd', '\x77',
  '\x96', '\x65', '\x7d', '\x36', '\x0c', '\x43', '\xe4', '\x72', '\x39', '\xe2',
  '\xf1', '\x38', '\xb7', '\xb7', '\xb7', '\xf4', '\xfb', '\x7d', '\x06', '\x83',
  '\x01', '\xa9', '\x54', '\xea', '\x43', '\xa5', '\x52', '\x39', '\x7e', '\x0a',
  '\x40', '\x29', '\x45', '\xab', '\xd5', '\x32', '\x4e', '\x4f', '\x4f', '\x7f',
  '\x4a', '\x29', '\xd5', '\xdd', '\xdd', '\xdd', '\xb3', '\xea', '\xf5', '\x7a',
  '\xea', '\xe4', '\xe4', '\x44', '\x49', '\x29', '\xd3', '\x2b', '\xcf', '\xaa',
  '\x36', '\x00', '\xc6', '\xe3', '\x71', '\x63', '\x67', '\x67', '\x47', '\xe4',
  '\xf3', '\x79', '\x00', '\x3c', '\xcf', '\xa3', '\x50', '\x28', '\xe0', '\xba',
  '\x2e', '\x9e', '\xe7', '\x91', '\x4a', '\xa5', '\xd8', '\xdb', '\xdb', '\x63',
  '\x34', '\x1a', '\x7d', '\x7c', '\x9a', '\xc1', '\x06', '\x80', '\x6d', '\xdb',
  '\xe9', '\x6c', '\x36', '\x8b', '\xcf', '\xe7', '\xc3', '\xf3', '\x3c', '\x5c',
  '\xd7', '\x05', '\x60', '\x36', '\x9b', '\xe1', '\xba', '\x2e', '\x8b', '\xc5',
  '\x82', '\x4c', '\x26', '\x83', '\x6d', '\xdb', '\xbb', '\x6b', '\x43', '\x74',
  '\x1c', '\xc7', '\x1f', '\x8b', '\xc5', '\x00', '\x28', '\x16', '\x8b', '\x8f',
  '\x9b', '\xd5', '\x6a', '\x15', '\x00', '\x29', '\x25', '\x9a', '\xa6', '\x31',
  '\x9f', '\xcf', '\xc5', '\x5a', '\xc0', '\xd6', '\xd6', '\x96', '\x9a', '\x4c',
  '\x26', '\x22', '\x14', '\x0a', '\x21', '\xa5', '\x64', '\x36', '\x9b', '\x51',
  '\xad', '\x56', '\xa9', '\xd5', '\x6a', '\x68', '\x9a', '\x06', '\xc0', '\xf5',
  '\xf5', '\x35', '\xba', '\xae', '\xdf', '\xaf', '\xbd', '\x82', '\x61', '\x18',
  '\xdf', '\x2c', '\xcb', '\x7a', '\x20', '\xfa', '\xfd', '\x04', '\x02', '\x01',
  '\x00', '\x34', '\x4d', '\x23', '\x10', '\x08', '\xb0', '\x58', '\x2c', '\xe8',
  '\x76', '\xbb', '\x24', '\x12', '\x89', '\x8b', '\xb5', '\x1d', '\x6c', '\x6f',
  '\x6f', '\x1f', '\x75', '\x3a', '\x9d', '\x7e', '\x38', '\x1c', '\x26', '\x97',
  '\xcb', '\x21', '\x84', '\x40', '\x4a', '\x09', '\xc0', '\xe6', '\xe6', '\x26',
  '\xed', '\x76', '\x9b', '\xc9', '\x64', '\xa2', '\xa6', '\xd3', '\xe9', '\x27',
  '\x78', '\x98', '\x9b', '\xd5', '\x53', '\x3e', '\xce', '\xc1', '\xf9', '\xf9',
  '\xf9', '\xfb', '\xc1', '\x60', '\x70', '\x9c', '\x4c', '\x26', '\xd9', '\xdf',
  '\xdf', '\x27', '\x12', '\x89', '\x30', '\x1c', '\x0e', '\xb9', '\xbc', '\xbc',
  '\xe4', '\xe6', '\xe6', '\x46', '\xf9', '\x7c', '\xbe', '\x2f', '\xd3', '\xe9',
  '\x34', '\x0f', '\x34', '\xea', '\xf5', '\x7a', '\xe5', '\x19', '\x60', '\x19',
  '\xd6', '\x9b', '\xd1', '\x68', '\xd4', '\x18', '\x0e', '\x87', '\xbb', '\xf3',
  '\xf9', '\x5c', '\xe8', '\xba', '\x7e', '\x6f', '\x18', '\xc6', '\x45', '\x34',
  '\x1a', '\x3d', '\x2c', '\x95', '\x4a', '\xdf', '\x4d', '\xd3', '\xbc', '\x02',
  '\x5e', '\x03', '\x5f', '\x81', '\x83', '\xb3', '\xb3', '\x33', '\xe7', '\x0f',
  '\xc0', '\xdf', '\x64', '\x9a', '\xa6', '\xb1', '\x34', '\xeb', '\x80', '\x03',
  '\x1c', '\x6c', '\xfc', '\x93', '\x73', '\xa9', '\x7a', '\xbd', '\x6e', '\x03',
  '\x47', '\xcb', '\xa5', '\x0e', '\xbc', '\xe2', '\x7f', '\x7e', '\xa3', '\x69',
  '\x9a', '\x87', '\xa6', '\x69', '\xbe', '\x55', '\x4a', '\x3d', '\x64', '\xf0',
  '\x12', '\xfd', '\x02', '\x0d', '\x53', '\x06', '\x24', '\x88', '\x3f', '\xe1',
  '\x69', '\x00', '\x00', '\x00', '\x00', '\x49', '\x45', '\x4e', '\x44', '\xae',
  '\x42', '\x60', '\x82',
};
