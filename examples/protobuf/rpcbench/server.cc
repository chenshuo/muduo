#include "examples/protobuf/rpcbench/echo.pb.h"

#include "muduo/base/Logging.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/protorpc/RpcServer.h"

#include <unistd.h>

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

}  // namespace echo

int main(int argc, char* argv[])
{
  int nThreads =  argc > 1 ? atoi(argv[1]) : 1;
  LOG_INFO << "pid = " << getpid() << " threads = " << nThreads;
  EventLoop loop;
  int port = argc > 2 ? atoi(argv[2]) : 8888;
  InetAddress listenAddr(static_cast<uint16_t>(port));
  echo::EchoServiceImpl impl;
  RpcServer server(&loop, listenAddr);
  server.setThreadNum(nThreads);
  server.registerService(&impl);
  server.start();
  loop.loop();
}

