#include "examples/curl/Curl.h"
#include "muduo/net/EventLoop.h"
#include <stdio.h>

using namespace muduo::net;

EventLoop* g_loop = NULL;

void onData(const char* data, int len)
{
  printf("len %d\n", len);
}

void done(curl::Request* c, int code)
{
  printf("done %p %s %d\n", c, c->getEffectiveUrl(), code);
}

void done2(curl::Request* c, int code)
{
  printf("done2 %p %s %d %d\n", c, c->getRedirectUrl(), c->getResponseCode(), code);
  // g_loop->quit();
}

int main(int argc, char* argv[])
{
  EventLoop loop;
  g_loop = &loop;
  loop.runAfter(30.0, std::bind(&EventLoop::quit, &loop));
  curl::Curl::initialize(curl::Curl::kCURLssl);
  curl::Curl curl(&loop);

  curl::RequestPtr req = curl.getUrl("http://chenshuo.com");
  req->setDataCallback(onData);
  req->setDoneCallback(done);

  curl::RequestPtr req2 = curl.getUrl("https://github.com");
  // req2->allowRedirect(5);
  req2->setDataCallback(onData);
  req2->setDoneCallback(done);

  curl::RequestPtr req3 = curl.getUrl("http://example.com");
  // req3->allowRedirect(5);
  req3->setDataCallback(onData);
  req3->setDoneCallback(done2);

  loop.loop();
}
