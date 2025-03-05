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

  typedef std::function<void(MariaDBClient*)> UpdateCallback; // similar to WriteCompleteCallback
  typedef std::function<void(MariaDBClient*, MYSQL_RES*)> QueryCallback;

  enum State
  {
    kRealConnectStart,
    kRealConnectCont,
    kRealConnectEnd,

    kRealQueryStart,
    kRealQueryCont,
    kRealQueryEnd,

    kStoreResultStart,
    kStoreResultCont,
    kStoreResultEnd,

    kCloseStart,
    kCloseCont,
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

  // INSERT, UPDATE, or DELETE
  void executeUpdate(muduo::StringArg sql, const UpdateCallback& cb = UpdateCallback());
  // SELECT, SHOW, DESCRIBE, EXPLAIN, CHECK TABLE, and so forth
  void executeQuery(muduo::StringArg sql, const QueryCallback& cb = QueryCallback());

  uint32_t errorNo() { return ::mysql_errno(&mysql_); }
  const char* errorStr() { return ::mysql_error(&mysql_); }

 private:
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

  struct SQLData
  {
    enum Type
    {
      kUpdate,
      kQuery
    };

    SQLData(Type type, const string& sql, const UpdateCallback& cb)
        : type_(type), sql_(sql), updateCb_(cb) {}
    SQLData(Type type, const string& sql, const QueryCallback& cb)
        : type_(type), sql_(sql), queryCb_(cb) {}
    ~SQLData() {}

    Type type_;
    string sql_;
    union
    {
      UpdateCallback updateCb_;
      QueryCallback queryCb_;
    };
  };

  std::deque<std::unique_ptr<SQLData>> sqlQueue_;
};

}  // namespace mariadbclient

#endif  // MUDUO_CONTRIB_MARIADBCLIENT_MARIADBCLIENT_H
