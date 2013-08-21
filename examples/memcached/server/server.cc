#include "MemcacheServer.h"

#include <muduo/net/EventLoop.h>
#include <muduo/net/EventLoopThread.h>
#include <muduo/net/inspect/Inspector.h>

#include <boost/program_options.hpp>

namespace po = boost::program_options;
using namespace muduo::net;

bool parseCommandLine(int argc, char* argv[], MemcacheServer::Options* options)
{
  options->tcpport = 11211;
  options->gperfport = 11212;
  options->threads = 4;

  po::options_description desc("Allowed options");
  desc.add_options()
      ("help,h", "Help")
      ("port,p", po::value<uint16_t>(&options->tcpport), "TCP port")
      ("udpport,U", po::value<uint16_t>(&options->udpport), "UDP port")
      ("gperf,g", po::value<uint16_t>(&options->gperfport), "port for gperftools")
      ("threads,t", po::value<int>(&options->threads), "Number of worker threads")
      ;

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help"))
  {
    //printf("memcached 1.1.0\n");
    return false;
  }
  return true;
}

int main(int argc, char* argv[])
{
  EventLoop loop;
  EventLoopThread inspectThread;
  MemcacheServer::Options options;
  if (parseCommandLine(argc, argv, &options))
  {
    // FIXME: how to destruct it safely ?
    new Inspector(inspectThread.startLoop(), InetAddress(options.gperfport), "memcached-debug");

    MemcacheServer server(&loop, options);
    server.setThreadNum(options.threads);
    server.start();
    loop.loop();
  }
}

