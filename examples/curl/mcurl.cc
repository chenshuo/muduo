#include <examples/curl/Curl.h>
#include <muduo/net/EventLoop.h>
#include <stdio.h>

using namespace muduo::net;

EventLoop* g_loop = NULL;

void onData(const char* data, int len)
{
  printf("len %d\n", len);
}

void done(CURL* c, int code)
{
  printf("done %p %d\n", c, code);
  g_loop->quit();
}

int main(int argc, char* argv[])
{
  EventLoop loop;
  g_loop = &loop;
  curl::Curl::initialize();
  curl::Curl curl(&loop);

  curl::RequestPtr req = curl.getUrl("http://chenshuo.com");
  req->setDataCallback(onData);
  req->setDoneCallback(done);

  loop.loop();
}
