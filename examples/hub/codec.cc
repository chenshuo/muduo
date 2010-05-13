#include "codec.h"

using namespace muduo;
using namespace muduo::net;
using namespace pubsub;

void PubSubCodec::onMessage(const TcpConnectionPtr& conn,
                            Buffer* buf,
                            Timestamp receiveTime)
{
  //ClientContext* context = &boost::any_cast<ClientContext&>(conn->getContext());
  const char* crlf = buf->findCRLF();
  if (crlf)
  {
  }
}

