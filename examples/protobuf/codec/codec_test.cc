#include "codec.h"
#include <examples/protobuf/codec/query.pb.h>

#include <stdio.h>
#include <zlib.h>  // adler32

using namespace muduo;
using namespace muduo::net;

void print(const Buffer& buf)
{
  printf("encoded to %zd bytes\n", buf.readableBytes());
  for (size_t i = 0; i < buf.readableBytes(); ++i)
  {
    unsigned char ch = static_cast<unsigned char>(buf.peek()[i]);

    printf("%2zd:  0x%02x  %c\n", i, ch, isgraph(ch) ? ch : ' ');
  }
}

void testQuery()
{
  muduo::Query query;
  query.set_id(1);
  query.set_sender("Chen Shuo");
  query.add_question("Running?");

  Buffer buf;
  ProtobufCodec::fillEmptyBuffer(&buf, query);
  print(buf);

  const int32_t len = buf.readInt32();
  assert(len == static_cast<int32_t>(buf.readableBytes()));

  ProtobufCodec::ErrorCode errorCode = ProtobufCodec::kNoError;
  MessagePtr message = ProtobufCodec::parse(buf.peek(), len, &errorCode);
  assert(errorCode == ProtobufCodec::kNoError);
  assert(message != NULL);
  message->PrintDebugString();
  assert(message->DebugString() == query.DebugString());

  boost::shared_ptr<muduo::Query> newQuery = boost::dynamic_pointer_cast<muduo::Query>(message);
  assert(newQuery != NULL);
}

void testAnswer()
{
  muduo::Answer answer;
  answer.set_id(1);
  answer.set_sender("Chen Shuo");
  answer.set_answerer("blog.csdn.net/Solstice");
  answer.add_solution("Jump!");
  answer.add_solution("Win!");

  Buffer buf;
  ProtobufCodec::fillEmptyBuffer(&buf, answer);
  print(buf);

  const int32_t len = buf.readInt32();
  assert(len == static_cast<int32_t>(buf.readableBytes()));

  ProtobufCodec::ErrorCode errorCode = ProtobufCodec::kNoError;
  MessagePtr message = ProtobufCodec::parse(buf.peek(), len, &errorCode);
  assert(errorCode == ProtobufCodec::kNoError);
  assert(message != NULL);
  message->PrintDebugString();
  assert(message->DebugString() == answer.DebugString());

  boost::shared_ptr<muduo::Answer> newAnswer = boost::dynamic_pointer_cast<muduo::Answer>(message);
  assert(newAnswer != NULL);
}

void testEmpty()
{
  muduo::Empty empty;

  Buffer buf;
  ProtobufCodec::fillEmptyBuffer(&buf, empty);
  print(buf);

  const int32_t len = buf.readInt32();
  assert(len == static_cast<int32_t>(buf.readableBytes()));

  ProtobufCodec::ErrorCode errorCode = ProtobufCodec::kNoError;
  MessagePtr message = ProtobufCodec::parse(buf.peek(), len, &errorCode);
  assert(message != NULL);
  message->PrintDebugString();
  assert(message->DebugString() == empty.DebugString());
}

void redoCheckSum(string& data, int len)
{
  int32_t checkSum = ::htonl(static_cast<int32_t>(
      ::adler32(0,
                reinterpret_cast<const Bytef*>(data.c_str()),
                static_cast<int>(len - 4))));
  data[len-4] = reinterpret_cast<const char*>(&checkSum)[0];
  data[len-3] = reinterpret_cast<const char*>(&checkSum)[1];
  data[len-2] = reinterpret_cast<const char*>(&checkSum)[2];
  data[len-1] = reinterpret_cast<const char*>(&checkSum)[3];
}

void testBadBuffer()

{
  muduo::Empty empty;
  empty.set_id(43);

  Buffer buf;
  ProtobufCodec::fillEmptyBuffer(&buf, empty);
  // print(buf);

  const int32_t len = buf.readInt32();
  assert(len == static_cast<int32_t>(buf.readableBytes()));


  {
    string data(buf.peek(), len);
    ProtobufCodec::ErrorCode errorCode = ProtobufCodec::kNoError;
    MessagePtr message = ProtobufCodec::parse(data.c_str(), len-1, &errorCode);
    assert(message == NULL);
    assert(errorCode == ProtobufCodec::kCheckSumError);
  }

  {
    string data(buf.peek(), len);
    ProtobufCodec::ErrorCode errorCode = ProtobufCodec::kNoError;
    data[len-1]++;
    MessagePtr message = ProtobufCodec::parse(data.c_str(), len, &errorCode);
    assert(message == NULL);
    assert(errorCode == ProtobufCodec::kCheckSumError);
  }

  {
    string data(buf.peek(), len);
    ProtobufCodec::ErrorCode errorCode = ProtobufCodec::kNoError;
    data[0]++;
    MessagePtr message = ProtobufCodec::parse(data.c_str(), len, &errorCode);
    assert(message == NULL);
    assert(errorCode == ProtobufCodec::kCheckSumError);
  }

  {
    string data(buf.peek(), len);
    ProtobufCodec::ErrorCode errorCode = ProtobufCodec::kNoError;
    data[3] = 0;
    redoCheckSum(data, len);
    MessagePtr message = ProtobufCodec::parse(data.c_str(), len, &errorCode);
    assert(message == NULL);
    assert(errorCode == ProtobufCodec::kInvalidNameLen);
  }

  {
    string data(buf.peek(), len);
    ProtobufCodec::ErrorCode errorCode = ProtobufCodec::kNoError;
    data[3] = 100;
    redoCheckSum(data, len);
    MessagePtr message = ProtobufCodec::parse(data.c_str(), len, &errorCode);
    assert(message == NULL);
    assert(errorCode == ProtobufCodec::kInvalidNameLen);
  }

  {
    string data(buf.peek(), len);
    ProtobufCodec::ErrorCode errorCode = ProtobufCodec::kNoError;
    data[3]--;
    redoCheckSum(data, len);
    MessagePtr message = ProtobufCodec::parse(data.c_str(), len, &errorCode);
    assert(message == NULL);
    assert(errorCode == ProtobufCodec::kUnknownMessageType);
  }

  {
    string data(buf.peek(), len);
    ProtobufCodec::ErrorCode errorCode = ProtobufCodec::kNoError;
    data[4] = 'M';
    redoCheckSum(data, len);
    MessagePtr message = ProtobufCodec::parse(data.c_str(), len, &errorCode);
    assert(message == NULL);
    assert(errorCode == ProtobufCodec::kUnknownMessageType);
  }

  {
    // FIXME: reproduce parse error
    string data(buf.peek(), len);
    ProtobufCodec::ErrorCode errorCode = ProtobufCodec::kNoError;
    redoCheckSum(data, len);
    MessagePtr message = ProtobufCodec::parse(data.c_str(), len, &errorCode);
    // assert(message == NULL);
    // assert(errorCode == ProtobufCodec::kParseError);
  }
}

int main()
{
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  testQuery();
  puts("");
  testAnswer();
  puts("");
  testEmpty();
  puts("");
  testBadBuffer();
  puts("");

  puts("All pass!!!");

  google::protobuf::ShutdownProtobufLibrary();
}

