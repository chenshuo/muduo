#include "tunnel.h"

#include <stdio.h>

using namespace muduo;
using namespace muduo::net;

MutexLock g_mutex;
size_t g_current = 0;
std::vector<InetAddress> g_backends;
std::map<string, TunnelPtr> g_tunnels;

void onServerConnection(const TcpConnectionPtr& conn)
{
  LOG_DEBUG << (conn->connected() ? "UP" : "DOWN");
  if (conn->connected())
  {
    conn->setTcpNoDelay(true);
    MutexLockGuard guard(g_mutex);
    InetAddress backend = g_backends[g_current];
    g_current = (g_current+1) % g_backends.size();

    TunnelPtr tunnel(new Tunnel(conn->getLoop(), backend, conn));
    tunnel->setup();
    tunnel->connect();

    g_tunnels[conn->name()] = tunnel;
  }
  else
  {
    MutexLockGuard guard(g_mutex);
    assert(g_tunnels.find(conn->name()) != g_tunnels.end());
    g_tunnels[conn->name()]->disconnect();
    g_tunnels.erase(conn->name());
  }
}

void onServerMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp)
{
  if (!conn->getContext().empty())
  {
    const TcpConnectionPtr& clientConn
      = boost::any_cast<const TcpConnectionPtr&>(conn->getContext());
    clientConn->send(buf);
  }
}

int main(int argc, char* argv[])
{
  if (argc < 3)
  {
    fprintf(stderr, "Usage: %s listen_port backend_ip:port [backend_ip:port]\n", argv[0]);
  }
  else
  {
    for (int i = 2; i < argc; ++i)
    {
      string hostport = argv[i];
      size_t colon = hostport.find(':');
      if (colon != string::npos)
      {
        string ip = hostport.substr(0, colon);
        uint16_t port = static_cast<uint16_t>(atoi(hostport.c_str()+colon+1));
        g_backends.push_back(InetAddress(ip, port));
      }
      else
      {
        fprintf(stderr, "invalid backend address %s\n", argv[i]);
        return 1;
      }
    }

    uint16_t port = static_cast<uint16_t>(atoi(argv[1]));
    InetAddress listenAddr(port);

    EventLoop loop;
    TcpServer server(&loop, listenAddr, "TcpBalancer");
    server.setConnectionCallback(onServerConnection);
    server.setMessageCallback(onServerMessage);
    server.setThreadNum(4);
    server.start();
    loop.loop();
  }
}

