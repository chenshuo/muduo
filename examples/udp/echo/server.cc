
#include <stdio.h> 
#include <boost/bind.hpp>

#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>

#include <muduo/net/udp/UdpServer.h>

using namespace muduo;
using namespace muduo::net;

void onMessage(
            muduo::net::EventLoop* loop,
            const UdpServerPtr& server,
            UdpMessagePtr& udpMsg,
            Timestamp timestamp)
{
  muduo::net::UdpMessage::send(udpMsg.get());
  muduo::string msg(udpMsg->data(), udpMsg->size());
  LOG_INFO << "recv msg:" << msg << " loop tid=" << muduo::CurrentThread::tid();
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
  muduo::net::EventLoop loop;
  muduo::net::InetAddress remoteAddr(host, port);
  muduo::net::UdpServerPtr server(new muduo::net::UdpServer(&loop, remoteAddr, "echo"));
  server->setMessageCallback(&onMessage);
  server->setThreadNum(10);
  server->start();
  loop.loop();
  return 0;
}

