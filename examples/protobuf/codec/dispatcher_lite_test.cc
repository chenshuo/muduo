#include "examples/protobuf/codec/dispatcher_lite.h"

#include "examples/protobuf/codec/query.pb.h"

#include <iostream>

using std::cout;
using std::endl;

void onUnknownMessageType(const muduo::net::TcpConnectionPtr&,
                          const MessagePtr& message,
                          muduo::Timestamp)
{
  cout << "onUnknownMessageType: " << message->GetTypeName() << endl;
}

void onQuery(const muduo::net::TcpConnectionPtr&,
             const MessagePtr& message,
             muduo::Timestamp)
{
  cout << "onQuery: " << message->GetTypeName() << endl;
  std::shared_ptr<muduo::Query> query = muduo::down_pointer_cast<muduo::Query>(message);
  assert(query != NULL);
}

void onAnswer(const muduo::net::TcpConnectionPtr&,
              const MessagePtr& message,
              muduo::Timestamp)
{
  cout << "onAnswer: " << message->GetTypeName() << endl;
  std::shared_ptr<muduo::Answer> answer = muduo::down_pointer_cast<muduo::Answer>(message);
  assert(answer != NULL);
}

int main()
{
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  ProtobufDispatcherLite dispatcher(onUnknownMessageType);
  dispatcher.registerMessageCallback(muduo::Query::descriptor(), onQuery);
  dispatcher.registerMessageCallback(muduo::Answer::descriptor(), onAnswer);

  muduo::net::TcpConnectionPtr conn;
  muduo::Timestamp t;

  std::shared_ptr<muduo::Query> query(new muduo::Query);
  std::shared_ptr<muduo::Answer> answer(new muduo::Answer);
  std::shared_ptr<muduo::Empty> empty(new muduo::Empty);
  dispatcher.onProtobufMessage(conn, query, t);
  dispatcher.onProtobufMessage(conn, answer, t);
  dispatcher.onProtobufMessage(conn, empty, t);

  google::protobuf::ShutdownProtobufLibrary();
}

