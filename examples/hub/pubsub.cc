#include "pubsub.h"

using namespace muduo;
using namespace muduo::net;
using namespace pubsub;

PubSubClient::PubSubClient(EventLoop* loop,
                           const InetAddress& hubAddr,
                           const string& name)
  : loop_(loop),
    client_(loop, hubAddr, name)
{
}

void PubSubClient::start()
{
  client_.connect();
}

void PubSubClient::stop()
{
  client_.disconnect();
}



