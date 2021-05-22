#include "MariaDBClient.h"

#include <muduo/base/Logging.h>
#include <muduo/net/Channel.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/SocketsOps.h>

#include <poll.h>

#define NEXT_IMMEDIATE(newState) do { state = newState; goto again; } while (0)

using namespace mariadbclient;
using namespace muduo;
using namespace muduo::net;

static void dummy(const std::shared_ptr<Channel>&)
{
}

MariaDBClient::MariaDBClient(EventLoop* loop,
                             const InetAddress& serverAddr,
                             const string& user,
                             const string& password,
                             const string& db)
    : loop_(loop),
      serverAddr_(serverAddr),
      user_(user),
      password_(password),
      db_(db),
      isConnected_(false)
{
}

MariaDBClient::~MariaDBClient()
{
  if (isConnected_)
  {
    disconnect();
  }
}

void MariaDBClient::connect()
{
  assert(!isConnected_);

  ::mysql_init(&mysql_);
  ::mysql_options(&mysql_, MYSQL_OPT_NONBLOCK, 0);

  stateMachineHandler(kRealConnectStart);
}

void MariaDBClient::disconnect()
{
  assert(isConnected_);

  stateMachineHandler(kCloseStart);
}

void MariaDBClient::executeUpdate(StringArg sql, const UpdateCallback& cb)
{
  assert(isConnected_);

  sqlQueue_.emplace_back(new SQLData(SQLData::kUpdate, sql.c_str(), cb));

  if (sqlQueue_.size() == 1)
  {
    stateMachineHandler(kRealQueryStart);
  }
}

void MariaDBClient::executeQuery(StringArg sql, const QueryCallback& cb)
{
  assert(isConnected_);

  sqlQueue_.emplace_back(new SQLData(SQLData::kQuery, sql.c_str(), cb));

  if (sqlQueue_.size() == 1)
  {
    stateMachineHandler(kRealQueryStart);
  }
}

void MariaDBClient::stateMachineHandler(int state, int revents, Timestamp receiveTime)
{
  loop_->assertInLoopThread();

  int mysqlRevents = toMySQLEvents(revents);

  static MYSQL* ret = NULL;
  static int err = 0;
  static MYSQL_RES* res = NULL;

  again:
  switch (state)
  {
    case kRealConnectStart:
    {
      int mysqlEvents = ::mysql_real_connect_start(&ret,
                                                   &mysql_,
                                                   serverAddr_.toIp().c_str(),
                                                   user_.c_str(),
                                                   password_.c_str(),
                                                   db_.c_str(),
                                                   implicit_cast<uint32_t>(serverAddr_.toPort()),
                                                   NULL,
                                                   0);
      if (mysqlEvents != 0)
      {
        channel_.reset(new Channel(loop_, fd(), false));

        channel_->setEventsCallback(std::bind(&MariaDBClient::stateMachineHandler, this, kRealConnectCont, _1, _2));
        int events = toEvents(mysqlEvents);
        channel_->enableEvents(events);
      }
      else
      {
        NEXT_IMMEDIATE(kRealConnectEnd);
      }
    }
      break;

    case kRealConnectCont:
    {
      int mysqlEvents = ::mysql_real_connect_cont(&ret, &mysql_, mysqlRevents);
      if (mysqlEvents != 0)
      {
        int events = toEvents(mysqlEvents);
        channel_->enableEvents(events);
      }
      else
      {
        NEXT_IMMEDIATE(kRealConnectEnd);
      }
    }
      break;

    case kRealConnectEnd:
    {
      if (ret == NULL)
      {
        LOG_ERROR << "Failed to mysql_real_connect(): " << errorStr();
      }
      else
      {
        logConnection(true);
        isConnected_ = true;
        channel_->setEventsCallback(Channel::EventsCallback());
        channel_->disableAll();
      }

      if (connectCb_)
      {
        connectCb_(this);
      }
    }
      break;

    case kRealQueryStart:
    {
      int mysqlEvents = ::mysql_real_query_start(&err,
                                                 &mysql_,
                                                 sqlQueue_.front()->sql_.c_str(),
                                                 sqlQueue_.front()->sql_.size());
      if (mysqlEvents != 0)
      {
        channel_->setEventsCallback(std::bind(&MariaDBClient::stateMachineHandler, this, kRealQueryCont, _1, _2));
        int events = toEvents(mysqlEvents);
        channel_->enableEvents(events);
      }
      else
      {
        NEXT_IMMEDIATE(kRealQueryEnd);
      }
    }
      break;

    case kRealQueryCont:
    {
      int mysqlEvents = ::mysql_real_query_cont(&err, &mysql_, mysqlRevents);
      if (mysqlEvents != 0)
      {
        int events = toEvents(mysqlEvents);
        channel_->enableEvents(events);
      }
      else
      {
        NEXT_IMMEDIATE(kRealQueryEnd);
      }
    }
      break;

    case kRealQueryEnd:
    {
      if (sqlQueue_.front()->type_ == SQLData::kUpdate)
      {
        if (sqlQueue_.front()->updateCb_)
        {
          sqlQueue_.front()->updateCb_(this);
        }
        sqlQueue_.pop_front();

        if (!sqlQueue_.empty())
        {
          NEXT_IMMEDIATE(kRealQueryStart);
        }
      }
      else
      {
        assert(sqlQueue_.front()->type_ == SQLData::kQuery);
        if (err != 0)
        {
          LOG_ERROR << "mysql_real_query() returns error: " << errorStr();
          if (sqlQueue_.front()->queryCb_)
          {
            sqlQueue_.front()->queryCb_(this, NULL);
          }
          sqlQueue_.pop_front();

          if (!sqlQueue_.empty())
          {
            NEXT_IMMEDIATE(kRealQueryStart);
          }
        }
        else
        {
          NEXT_IMMEDIATE(kStoreResultStart);
        }
      }
    }
      break;

    case kStoreResultStart:
    {
      int mysqlEvents = ::mysql_store_result_start(&res, &mysql_);
      if (mysqlEvents != 0)
      {
        channel_->setEventsCallback(std::bind(&MariaDBClient::stateMachineHandler, this, kStoreResultCont, _1, _2));

        int events = toEvents(mysqlEvents);
        channel_->enableEvents(events);
      }
      else
      {
        NEXT_IMMEDIATE(kStoreResultEnd);
      }
    }
      break;

    case kStoreResultCont:
    {
      int mysqlEvents = ::mysql_store_result_cont(&res, &mysql_, mysqlRevents);
      if (mysqlEvents != 0)
      {
        int events = toEvents(mysqlEvents);
        channel_->enableEvents(events);
      }
      else
      {
        NEXT_IMMEDIATE(kStoreResultEnd);
      }
    }
      break;

    case kStoreResultEnd:
    {
      if (sqlQueue_.front()->queryCb_)
      {
        sqlQueue_.front()->queryCb_(this, res);
      }
      else
      {
        if (res != NULL)
        {
          ::mysql_free_result(res);
        }
        else
        {
          assert(::mysql_field_count(&mysql_) != 0);
          assert(::mysql_errno(&mysql_) != 0);
          LOG_ERROR << "Got error while storing result: " << errorStr();
        }
      }
      sqlQueue_.pop_front();

      if (!sqlQueue_.empty())
      {
        NEXT_IMMEDIATE(kRealQueryStart);
      }
    }
      break;

    case kCloseStart:
    {
      logConnection(false);
      int mysqlEvents = ::mysql_close_start(&mysql_);
      if (mysqlEvents != 0)
      {
        channel_->setEventsCallback(std::bind(&MariaDBClient::stateMachineHandler, this, kCloseCont, _1, _2));
        int events = toEvents(mysqlEvents);
        channel_->enableEvents(events);
      }
      else
      {
        NEXT_IMMEDIATE(kCloseEnd);
      }
    }
      break;

    case kCloseCont:
    {
      int mysqlEvents = ::mysql_close_cont(&mysql_, mysqlRevents);
      if (mysqlEvents != 0)
      {
        int events = toEvents(mysqlEvents);
        channel_->enableEvents(events);
      }
      else
      {
        NEXT_IMMEDIATE(kCloseEnd);
      }
    }
      break;

    case kCloseEnd:
    {
      isConnected_ = false;
      channel_->disableAll();
      channel_->remove();
      loop_->queueInLoop(std::bind(dummy, channel_));
      channel_.reset();

      if (disconnectCb_)
      {
        disconnectCb_(this);
      }
    }
      break;

    default:
    {
      abort();
    }
  }
}

void MariaDBClient::logConnection(bool up) const
{
  InetAddress localAddr(sockets::getLocalAddr(fd()));
  InetAddress peerAddr(sockets::getPeerAddr(fd()));

  LOG_INFO << localAddr.toIpPort() << " -> "
           << peerAddr.toIpPort() << " is "
           << (up ? "UP" : "DOWN");
}

int MariaDBClient::toEvents(int mysqlEvents)
{
  int events = (mysqlEvents & MYSQL_WAIT_READ ? POLLIN : 0)
      | (mysqlEvents & MYSQL_WAIT_WRITE ? POLLOUT : 0)
      | (mysqlEvents & MYSQL_WAIT_EXCEPT ? POLLPRI : 0);

  return events;
}

int MariaDBClient::toMySQLEvents(int events)
{
  int mysqlEvents = (events & POLLIN ? MYSQL_WAIT_READ : 0)
      | (events & POLLPRI ? MYSQL_WAIT_EXCEPT : 0)
      | (events & POLLOUT ? MYSQL_WAIT_WRITE : 0);
  return mysqlEvents;
}
