#include <examples/protobuf/rpcbench/echo.pb.h>

#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/protorpc/RpcServer.h>

#include <boost/bind.hpp>

using namespace muduo;
using namespace muduo::net;

namespace echo
{

class EchoServiceImpl : public EchoService
{
 public:
  virtual void Echo(::google::protobuf::RpcController* controller,
                    const ::echo::EchoRequest* request,
                    ::echo::EchoResponse* response,
                    ::google::protobuf::Closure* done)
  {
    //LOG_INFO << "EchoServiceImpl::Solve";
    response->set_payload(request->payload());
    done->Run();
  }
};

}

int main()
{
  LOG_INFO << "pid = " << getpid();
  EventLoop loop;
  InetAddress listenAddr(8888);
  echo::EchoServiceImpl impl;
  RpcServer server(&loop, listenAddr);
  server.setThreadNum(2);
  server.registerService(&impl);
  server.start();
  loop.loop();
}

