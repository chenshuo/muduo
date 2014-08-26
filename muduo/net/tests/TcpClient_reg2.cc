// TcpClient destructs when TcpConnection is connected but unique.

#include <muduo/base/Logging.h>
#include <muduo/base/Thread.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpClient.h>

#include <boost/bind.hpp>

using namespace muduo;
using namespace muduo::net;

void threadFunc(EventLoop* loop)
{
  InetAddress serverAddr("127.0.0.1", 1234); // should succeed
  TcpClient client(loop, serverAddr, "TcpClient");
  client.connect();

  CurrentThread::sleepUsec(1000*1000);
  // client destructs when connected.
}

int main(int argc, char* argv[])
{
  Logger::setLogLevel(Logger::DEBUG);

  EventLoop loop;
  loop.runAfter(3.0, boost::bind(&EventLoop::quit, &loop));
  Thread thr(boost::bind(threadFunc, &loop));
  thr.start();
  loop.loop();
}
