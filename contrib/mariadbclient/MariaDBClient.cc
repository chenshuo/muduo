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
      isConnected_(false),
      res_(NULL),
      row_(NULL)
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
  loop_->runInLoop(std::bind(&MariaDBClient::connectInLoop, this));
}

void MariaDBClient::disconnect()
{
  loop_->runInLoop(std::bind(&MariaDBClient::disconnectInLoop, this));
}

void MariaDBClient::query(StringArg queryStr, const QueryCallback& cb)
{
  loop_->runInLoop(std::bind(&MariaDBClient::queryInLoop, this, string(queryStr.c_str()), cb));
}

void MariaDBClient::queryFetch(StringArg queryStr, const QueryFetchCallback& cb)
{
  loop_->runInLoop(std::bind(&MariaDBClient::queryFetchInLoop, this, string(queryStr.c_str()), cb));
}

void MariaDBClient::connectInLoop()
{
  loop_->assertInLoopThread();
  assert(!isConnected_);

  ::mysql_init(&mysql_);
  ::mysql_options(&mysql_, MYSQL_OPT_NONBLOCK, 0);

  stateMachineHandler(kRealConnectStart);
}

void MariaDBClient::disconnectInLoop()
{
  loop_->assertInLoopThread();
  assert(isConnected_);

  stateMachineHandler(kCloseStart);
}

void MariaDBClient::queryInLoop(StringArg queryStr, const QueryCallback& cb)
{
  loop_->assertInLoopThread();
  assert(isConnected_);

  queries_.emplace_back(new QueryData(QueryData::kQuery, queryStr.c_str(), cb));

  if (queries_.size() == 1)
  {
    stateMachineHandler(kRealQueryStart);
  }
}

void MariaDBClient::queryFetchInLoop(StringArg queryStr, const QueryFetchCallback& cb)
{
  loop_->assertInLoopThread();
  assert(isConnected_);

  queries_.emplace_back(new QueryData(QueryData::kQueryFetch, queryStr.c_str(), cb));

  if (queries_.size() == 1)
  {
    stateMachineHandler(kRealQueryStart);
  }
}

void MariaDBClient::stateMachineHandler(int state, int revents, Timestamp receiveTime)
{
  loop_->assertInLoopThread();

  int mysqlRevents = toMySQLEvents(revents);
  MYSQL* ret = NULL;
  int err = 0;

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
      if (!ret)
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
      int mysqlEvents =
          ::mysql_real_query_start(&err,
                                   &mysql_,
                                   queries_.front()->queryStr_.c_str(),
                                   queries_.front()->queryStr_.size());
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
      if (queries_.front()->type_ == QueryData::kQuery)
      {
        if (queries_.front()->queryCb_)
        {
          queries_.front()->queryCb_(this);
        }
        queries_.pop_front();

        if (!queries_.empty())
        {
          NEXT_IMMEDIATE(kRealQueryStart);
        }
      }
      else
      {
        assert(queries_.front()->type_ == QueryData::kQueryFetch);
        assert(queries_.front()->queryFetchCb_);
        if (err)
        {
          LOG_ERROR << "mysql_real_query() returns error: " << errorStr();
          queries_.front()->queryFetchCb_(this, FetchResultPtr());
        }
        else
        {
          res_ = ::mysql_use_result(&mysql_);
          if (!res_)
          {
            LOG_ERROR << "mysql_use_result() returns error: " << errorStr();
            queries_.front()->queryFetchCb_(this, FetchResultPtr());
          }
          else
          {
            result_.reset(new FetchResult);
            NEXT_IMMEDIATE(kFetchRowStart);
          }
        }
      }
    }
      break;

    case kFetchRowStart:
    {
      int mysqlEvents = ::mysql_fetch_row_start(&row_, res_);
      if (mysqlEvents != 0)
      {
        channel_->setEventsCallback(std::bind(&MariaDBClient::stateMachineHandler, this, kFetchRowCont, _1, _2));

        int events = toEvents(mysqlEvents);
        channel_->enableEvents(events);
      }
      else
      {
        NEXT_IMMEDIATE(kFetchRowEnd);
      }
    }
      break;

    case kFetchRowCont:
    {
      int mysqlEvents = ::mysql_fetch_row_cont(&row_, res_, mysqlRevents);
      if (mysqlEvents != 0)
      {
        int events = toEvents(mysqlEvents);
        channel_->enableEvents(events);
      }
      else
      {
        NEXT_IMMEDIATE(kFetchRowEnd);
      }
    }
      break;

    case kFetchRowEnd:
    {
      if (row_)
      {
        std::vector<string> fields;
        for (size_t i = 0; i < ::mysql_num_fields(res_); ++i)
        {
          if (row_[i])
          {
            fields.push_back(row_[i]);
          }
          else
          {
            fields.push_back("");
          }
        }
        result_->push_back(fields);

        NEXT_IMMEDIATE(kFetchRowStart);
      }
      else
      {
        if (errorNo() != 0)
        {
          LOG_ERROR << "Got error while retrieving rows: " << errorStr();
        }
        ::mysql_free_result(res_);

        assert(queries_.front()->queryFetchCb_);
        queries_.front()->queryFetchCb_(this, result_);
        queries_.pop_front();
        result_.reset();

        if (!queries_.empty())
        {
          NEXT_IMMEDIATE(kRealQueryStart);
        }
      }
    }
      break;

    case kCloseStart:
    {
      logConnection(false);
      int mysqlEvents = ::mysql_close_start(&mysql_);
      if (mysqlEvents != 0)
      {
        channel_->setEventsCallback(std::bind(&MariaDBClient::stateMachineHandler, this, kCloseContinue, _1, _2));
        int events = toEvents(mysqlEvents);
        channel_->enableEvents(events);
      }
      else
      {
        NEXT_IMMEDIATE(kCloseEnd);
      }
    }
      break;

    case kCloseContinue:
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
      if (errorNo() != 0)
      {
        LOG_ERROR << "Got error while close: " << errorStr();
      }
      else
      {
        isConnected_ = false;
        channel_->disableAll();
        channel_->remove();
        loop_->queueInLoop(std::bind(dummy, channel_));
        channel_.reset();
      }

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
