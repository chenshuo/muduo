#include "examples/simple/chargen/chargen.h"

#include "muduo/base/Logging.h"
#include "muduo/net/EventLoop.h"

#include <unistd.h>

using namespace muduo;
using namespace muduo::net;

int main()
{
  LOG_INFO << "pid = " << getpid();
  EventLoop loop;
  InetAddress listenAddr(2019);
  ChargenServer server(&loop, listenAddr, true);
  server.start();
  loop.loop();
}

