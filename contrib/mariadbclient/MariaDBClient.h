#ifndef MUDUO_CONTRIB_MARIADBCLIENT_MARIADBCLIENT_H
#define MUDUO_CONTRIB_MARIADBCLIENT_MARIADBCLIENT_H

#include <muduo/base/noncopyable.h>
#include <muduo/base/Types.h>
#include <muduo/net/Channel.h>
#include <muduo/net/InetAddress.h>
#include <mysql/mysql.h>

#include <deque>
#include <functional>
#include <memory>
#include <vector>

namespace muduo
{
namespace net
{
class Channel;
class EventLoop;
}
}

namespace mariadbclient
{
using muduo::string;

class MariaDBClient : muduo::noncopyable
{
 public:
  typedef std::function<void(MariaDBClient*)> ConnectCallback;
  typedef std::function<void(MariaDBClient*)> DisconnectCallback;

  typedef std::function<void(MariaDBClient*)> QueryCallback;

  typedef std::vector<std::vector<string>> FetchResult;
  typedef std::shared_ptr<FetchResult> FetchResultPtr;
  typedef std::function<void(MariaDBClient*, const FetchResultPtr&)> QueryFetchCallback;

  enum State
  {
    kRealConnectStart,
    kRealConnectCont,
    kRealConnectEnd,

    kRealQueryStart,
    kRealQueryCont,
    kRealQueryEnd,

    kFetchRowStart,
    kFetchRowCont,
    kFetchRowEnd,

    kCloseStart,
    kCloseContinue,
    kCloseEnd,
  };

  MariaDBClient(muduo::net::EventLoop* loop,
                const muduo::net::InetAddress& serverAddr,
                const string& user,
                const string& password,
                const string& db);
  ~MariaDBClient();

  void setConnectCallback(const ConnectCallback& cb) { connectCb_ = cb; }
  void setDisconnectCallback(const DisconnectCallback& cb) { disconnectCb_ = cb; }

  void connect();
  void disconnect();

  void query(muduo::StringArg queryStr, const QueryCallback& cb = QueryCallback());
  void queryFetch(muduo::StringArg queryStr, const QueryFetchCallback& cb);

  uint32_t errorNo() { return ::mysql_errno(&mysql_); }
  const char* errorStr() { return ::mysql_error(&mysql_); }

 private:
  void connectInLoop();
  void disconnectInLoop();

  void queryInLoop(muduo::StringArg queryStr, const QueryCallback& cb = QueryCallback());
  void queryFetchInLoop(muduo::StringArg queryStr, const QueryFetchCallback& cb);

  void stateMachineHandler(int state, int revents = -1, muduo::Timestamp receiveTime = muduo::Timestamp::invalid());

  void logConnection(bool up) const;
  int fd() const { return ::mysql_get_socket(&mysql_); }

  static int toEvents(int mysqlEvents);
  static int toMySQLEvents(int events);

  muduo::net::EventLoop* loop_;
  const muduo::net::InetAddress serverAddr_;
  const string user_;
  const string password_;
  const string db_;
  std::shared_ptr<muduo::net::Channel> channel_;
  bool isConnected_;
  ConnectCallback connectCb_;
  DisconnectCallback disconnectCb_;

  MYSQL mysql_;

  struct QueryData
  {
    enum QueryType
    {
      kQuery,
      kQueryFetch
    };

    QueryData(QueryType type, const string& queryStr, const QueryCallback& cb)
        : type_(type), queryStr_(queryStr), queryCb_(cb) {}
    QueryData(QueryType type, const string& queryStr, const QueryFetchCallback& cb)
        : type_(type), queryStr_(queryStr), queryFetchCb_(cb) {}
    ~QueryData() {}

    QueryType type_;
    string queryStr_;
    union
    {
      QueryCallback queryCb_;
      QueryFetchCallback queryFetchCb_;
    };
  };

  std::deque<std::unique_ptr<QueryData>> queries_;

  MYSQL_RES* res_;
  MYSQL_ROW row_;
  FetchResultPtr result_;
};

}  // namespace mariadbclient

#endif  // MUDUO_CONTRIB_MARIADBCLIENT_MARIADBCLIENT_H
