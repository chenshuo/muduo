#undef NDEBUG
#include <muduo/net/protorpc/RpcCodec.h>
#include <muduo/net/protorpc/rpc.pb.h>
#include <muduo/net/protobuf/ProtobufCodecLite.h>
#include <muduo/net/Buffer.h>

#include <stdio.h>

using namespace muduo;
using namespace muduo::net;

void rpcMessageCallback(const TcpConnectionPtr&,
                        const RpcMessagePtr&,
                        Timestamp)
{
}

MessagePtr g_msgptr;
void messageCallback(const TcpConnectionPtr&,
                     const MessagePtr& msg,
                     Timestamp)
{
  g_msgptr = msg;
}

void print(const Buffer& buf)
{
  printf("encoded to %zd bytes\n", buf.readableBytes());
  for (size_t i = 0; i < buf.readableBytes(); ++i)
  {
    unsigned char ch = static_cast<unsigned char>(buf.peek()[i]);

    printf("%2zd:  0x%02x  %c\n", i, ch, isgraph(ch) ? ch : ' ');
  }
}

char rpctag[] = "RPC0";

int main()
{
  RpcMessage message;
  message.set_type(REQUEST);
  message.set_id(2);
  char wire[] = "\0\0\0\x13" "RPC0" "\x08\x01\x11\x02\0\0\0\0\0\0\0" "\x0f\xef\x01\x32";
  string expected(wire, sizeof(wire)-1);
  string s1, s2;
  Buffer buf1, buf2;
  {
  RpcCodec codec(rpcMessageCallback);
  codec.fillEmptyBuffer(&buf1, message);
  print(buf1);
  s1 = buf1.toStringPiece().as_string();
  }

  {
  ProtobufCodecLite codec(&RpcMessage::default_instance(), "RPC0", messageCallback);
  codec.fillEmptyBuffer(&buf2, message);
  print(buf2);
  s2 = buf2.toStringPiece().as_string();
  codec.onMessage(TcpConnectionPtr(), &buf1, Timestamp::now());
  assert(g_msgptr);
  assert(g_msgptr->DebugString() == message.DebugString());
  g_msgptr.reset();
  }
  assert(s1 == s2);
  assert(s1 == expected);
  assert(s2 == expected);

  {
  Buffer buf;
  ProtobufCodecLite codec(&RpcMessage::default_instance(), "XYZ", messageCallback);
  codec.fillEmptyBuffer(&buf, message);
  print(buf);
  s2 = buf.toStringPiece().as_string();
  codec.onMessage(TcpConnectionPtr(), &buf, Timestamp::now());
  assert(g_msgptr);
  assert(g_msgptr->DebugString() == message.DebugString());
  }

  google::protobuf::ShutdownProtobufLibrary();
}
