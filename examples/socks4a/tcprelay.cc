#include "tunnel.h"

#include <malloc.h>
#include <stdio.h>
#include <sys/resource.h>

using namespace muduo;
using namespace muduo::net;

EventLoop* g_eventLoop;
InetAddress* g_serverAddr;
std::map<string, TunnelPtr> g_tunnels;

void onServerConnection(const TcpConnectionPtr& conn)
{
  LOG_DEBUG << (conn->connected() ? "UP" : "DOWN");
  if (conn->connected())
  {
    conn->setTcpNoDelay(true);
    TunnelPtr tunnel(new Tunnel(g_eventLoop, *g_serverAddr, conn));
    tunnel->setup();
    tunnel->connect();
    g_tunnels[conn->name()] = tunnel;
  }
  else
  {
    assert(g_tunnels.find(conn->name()) != g_tunnels.end());
    g_tunnels[conn->name()]->disconnect();
    g_tunnels.erase(conn->name());
  }
}

void onServerMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp)
{
  LOG_DEBUG << buf->readableBytes();
  if (!conn->getContext().empty())
  {
    const TcpConnectionPtr& clientConn
      = boost::any_cast<const TcpConnectionPtr&>(conn->getContext());
    clientConn->send(buf);
  }
}

void memstat()
{
  malloc_stats();
}

int main(int argc, char* argv[])
{
  if (argc < 4)
  {
    fprintf(stderr, "Usage: %s <host_ip> <port> <listen_port>\n", argv[0]);
  }
  else
  {
    LOG_INFO << "pid = " << getpid() << ", tid = " << CurrentThread::tid();
    {
      // set max virtual memory to 256MB.
      size_t kOneMB = 1024*1024;
      rlimit rl = { 256*kOneMB, 256*kOneMB };
      setrlimit(RLIMIT_AS, &rl);
    }
    const char* ip = argv[1];
    uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
    InetAddress serverAddr(ip, port);
    g_serverAddr = &serverAddr;

    uint16_t acceptPort = static_cast<uint16_t>(atoi(argv[3]));
    InetAddress listenAddr(acceptPort);

    EventLoop loop;
    g_eventLoop = &loop;
    loop.runEvery(3, memstat);

    TcpServer server(&loop, listenAddr, "TcpRelay");

    server.setConnectionCallback(onServerConnection);
    server.setMessageCallback(onServerMessage);

    server.start();

    loop.loop();
  }
}

