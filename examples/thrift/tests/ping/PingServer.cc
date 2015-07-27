#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>

#include <thrift/protocol/TCompactProtocol.h>

#include "ThriftServer.h"

#include "Ping.h"

using namespace muduo;
using namespace muduo::net;

using apache::thrift::protocol::TCompactProtocolFactory;

using namespace ping;

class PingHandler : virtual public PingIf
{
 public:
  PingHandler()
  {
  }

  void ping()
  {
    LOG_INFO << "ping";
  }

};

int main(int argc, char **argv)
{
  EventLoop eventloop;
  InetAddress addr("127.0.0.1", 9090);
  string name("PingServer");

  boost::shared_ptr<PingHandler> handler(new PingHandler());
  boost::shared_ptr<TProcessor> processor(new PingProcessor(handler));
  boost::shared_ptr<TProtocolFactory> protcolFactory(new TCompactProtocolFactory());

  ThriftServer server(processor, protcolFactory, &eventloop, addr, name);
  server.start();
  eventloop.loop();

  return 0;
}

