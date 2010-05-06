#include <muduo/net/http/HttpServer.h>
#include <muduo/net/http/HttpRequest.h>
#include <muduo/net/http/HttpResponse.h>
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

  if (req.path() == "/")
  {
    resp->setStatusCode(HttpResponse::k200Ok);
    resp->setStatusMessage("OK");
    //resp->setCloseConnection(true);
    resp->setBody("<html><head><title>This is title</title></head><body><h1>Hello</h1></body></html>");
  }
  else
  {
    resp->setStatusCode(HttpResponse::k404NotFound);
    resp->setStatusMessage("Not Found");
    resp->setCloseConnection(true);
  }
}

int main()
{
  EventLoop loop;
  HttpServer server(&loop, InetAddress(8000), "dummy");
  server.setHttpCallback(onRequest);
  server.start();
  loop.loop();
}
