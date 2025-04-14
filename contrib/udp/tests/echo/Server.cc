
#include <stdio.h> 
#include <boost/bind.hpp>

#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>

#include <contrib/udp/UdpServer.h>

using namespace muduo;
using namespace muduo::net;
using namespace mudp;

void onMessage(
            EventLoop* loop,
            const UdpServerPtr& server,
            UdpMessagePtr& udpMsg,
            Timestamp timestamp)
{
  UdpMessage::send(udpMsg.get());
  string msg(udpMsg->data(), udpMsg->size());
  LOG_INFO << "recv msg:" << msg << " loop tid=" << CurrentThread::tid();
}

int main(int argc, char* argv[])
{
  if (argc != 2)
  {
    LOG_ERROR << "usage : " << argv[0] << " port";
    return 0;
  }
  const char* host = "0.0.0.0";
  uint16_t port = static_cast<uint16_t>(atoi(argv[1]));
  EventLoop loop;
  InetAddress remoteAddr(host, port);
  UdpServerPtr server(new UdpServer(&loop, remoteAddr, "echo"));
  server->setMessageCallback(&onMessage);
  server->setThreadNum(10);
  server->start();
  loop.loop();
  return 0;
}

