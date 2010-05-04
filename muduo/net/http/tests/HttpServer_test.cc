#include <muduo/net/http/HttpServer.h>
#include <muduo/net/http/HttpRequest.h>
#include <muduo/net/EventLoop.h>

#include <iostream>
#include <map>

using namespace muduo;
using namespace muduo::net;

void onRequest(const HttpRequest& req, HttpResponse* resp)
{
  std::cout << "Headers " << req.path() << std::endl;
  const std::map<string, string>& headers = req.headers();
  for (std::map<string, string>::const_iterator it = headers.begin();
       it != headers.end();
       ++it)
  {
    std::cout << it->first << ": " << it->second << std::endl;
  }

}

int main()
{
  EventLoop loop;
  HttpServer server(&loop, InetAddress(8000), "dummy");
  server.start();
  loop.loop();
}
