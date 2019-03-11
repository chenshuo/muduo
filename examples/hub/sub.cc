#include "examples/hub/pubsub.h"
#include "muduo/base/ProcessInfo.h"
#include "muduo/net/EventLoop.h"

#include <vector>
#include <stdio.h>

using namespace muduo;
using namespace muduo::net;
using namespace pubsub;

EventLoop* g_loop = NULL;
std::vector<string> g_topics;

void subscription(const string& topic, const string& content, Timestamp)
{
  printf("%s: %s\n", topic.c_str(), content.c_str());
}

void connection(PubSubClient* client)
{
  if (client->connected())
  {
    for (std::vector<string>::iterator it = g_topics.begin();
        it != g_topics.end(); ++it)
    {
      client->subscribe(*it, subscription);
    }
  }
  else
  {
    g_loop->quit();
  }
}

int main(int argc, char* argv[])
{
  if (argc > 2)
  {
    string hostport = argv[1];
    size_t colon = hostport.find(':');
    if (colon != string::npos)
    {
      string hostip = hostport.substr(0, colon);
      uint16_t port = static_cast<uint16_t>(atoi(hostport.c_str()+colon+1));
      for (int i = 2; i < argc; ++i)
      {
        g_topics.push_back(argv[i]);
      }

      EventLoop loop;
      g_loop = &loop;
      string name = ProcessInfo::username()+"@"+ProcessInfo::hostname();
      name += ":" + ProcessInfo::pidString();
      PubSubClient client(&loop, InetAddress(hostip, port), name);
      client.setConnectionCallback(connection);
      client.start();
      loop.loop();
    }
    else
    {
      printf("Usage: %s hub_ip:port topic [topic ...]\n", argv[0]);
    }
  }
  else
  {
    printf("Usage: %s hub_ip:port topic [topic ...]\n", argv[0]);
  }
}
