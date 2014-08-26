#include <examples/fastcgi/fastcgi.h>

#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>

#include <boost/bind.hpp>

using namespace muduo::net;

void onRequest(const TcpConnectionPtr& conn,
               FastCgiCodec::ParamMap& params,
               Buffer* in)
{
  LOG_INFO << conn->name() << ": " << params["REQUEST_URI"];

  for (FastCgiCodec::ParamMap::const_iterator it = params.begin();
      it != params.end(); ++it)
  {
    LOG_DEBUG << it->first << " = " << it->second;
  }
  Buffer response;
  response.append("Context-Type: text/plain\r\n\r\n");
  response.append("Hello FastCGI.");
  FastCgiCodec::respond(&response);
  conn->send(&response);
}

typedef boost::shared_ptr<FastCgiCodec> CodecPtr;
void onConnection(const TcpConnectionPtr& conn)
{
  if (conn->connected())
  {
    CodecPtr codec(new FastCgiCodec(onRequest));
    conn->setContext(codec);
    conn->setMessageCallback(
        boost::bind(&FastCgiCodec::onMessage, codec, _1, _2, _3));
  }
}

int main()
{
  muduo::net::EventLoop loop;
  TcpServer server(&loop, InetAddress(9000), "FastCGI");
  server.setConnectionCallback(onConnection);
  server.start();
  loop.loop();
}
