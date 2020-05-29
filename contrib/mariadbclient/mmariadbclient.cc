#include "MariaDBClient.h"

#include <muduo/base/CountDownLatch.h>
#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/EventLoopThread.h>

using namespace muduo;
using namespace muduo::net;

static const char* g_myGroups[] = {"client", NULL};
static uint64_t g_id = 0;

void connectCallback(mariadbclient::MariaDBClient* c, CountDownLatch* connected)
{
  if (c->errorNo() == 0)
  {
    LOG_INFO << "Connected... " << c->errorNo() << '\n' << c->errorStr();
    connected->countDown();
  }
  else
  {
    LOG_ERROR << "connectCallback Error: " << c->errorNo() << '\n' << c->errorStr();
  }
}

void disconnectCallback(mariadbclient::MariaDBClient* c, EventLoop* loop)
{
  if (c->errorNo() == 0)
  {
    LOG_INFO << "Disconnected... " << c->errorNo() << '\n' << c->errorStr();
  }
  else
  {
    LOG_ERROR << "disconnectCallback Error: " << c->errorNo() << '\n' << c->errorStr();
  }
  loop->quit();
}

void queryCallback(mariadbclient::MariaDBClient* c, uint64_t id)
{
  LOG_INFO << "message id: " << id << "\terrorNo: " << c->errorNo() << "\terrorStr: " << c->errorStr();
}

void selectCallback(mariadbclient::MariaDBClient* c,
                    const mariadbclient::MariaDBClient::FetchResultPtr& result,
                    uint64_t id)
{
  LOG_INFO << "message id: " << id << "\terrorNo: " << c->errorNo() << "\terrorStr: " << c->errorStr();
  LOG_INFO << "result size: " << result->size();
  if (!result->empty())
  {
    for (size_t i = 0; i < result->size(); ++i)
    {
      for (size_t j = 0; j < (*result)[i].size(); ++j)
      {
        printf("%s\t", (*result)[i][j].c_str());
      }
      printf("\n");
    }
  }
}

int main(int argc, char* argv[])
{
  int err = mysql_library_init(argc, argv, const_cast<char**>(g_myGroups));
  if (err)
  {
    LOG_FATAL << "mysql_library_init() returns error: " << err;
  }

  {
    EventLoop loop;
    EventLoopThread t;
    mariadbclient::MariaDBClient mariadbClient(t.startLoop(), InetAddress("127.0.0.1", 3306), "root", "123456", "test");
    CountDownLatch connected(1);
    mariadbClient.setConnectCallback(std::bind(&connectCallback, _1, &connected));
    mariadbClient.setDisconnectCallback(std::bind(&disconnectCallback, _1, &loop));
    mariadbClient.connect();
    connected.wait();

    mariadbClient.query("DROP TABLE muduo_user",
                        std::bind(&queryCallback, _1, g_id));

    mariadbClient.query("CREATE TABLE muduo_user (\
id INT(11) NOT NULL AUTO_INCREMENT,\
nick VARCHAR(64) NOT NULL,\
PRIMARY KEY (id)\
)",
                        std::bind(&queryCallback, _1, g_id++));

    mariadbClient.queryFetch("SELECT * FROM muduo_user",
                             std::bind(&selectCallback, _1, _2, g_id++));

    mariadbClient.query("INSERT INTO muduo_user \
(id, nick) \
VALUES (1, \"ChenShuo\")",
                        std::bind(&queryCallback, _1, g_id++));

    mariadbClient.query("INSERT INTO muduo_user \
(nick) \
VALUES (\"Jack\")");

    mariadbClient.query("INSERT INTO muduo_user \
(nick) \
VALUES (\"Lucy\")");

    mariadbClient.query("UPDATE muduo_user \
SET nick=\"Tom\" \
WHERE id=2",
                        std::bind(&queryCallback, _1, g_id++));

    mariadbClient.queryFetch("SELECT * FROM muduo_user",
                             std::bind(&selectCallback, _1, _2, g_id++));

    mariadbClient.query("DELETE FROM muduo_user \
WHERE id>1",
                        std::bind(&queryCallback, _1, g_id++));

    mariadbClient.queryFetch("SELECT * FROM muduo_user",
                             std::bind(&selectCallback, _1, _2, g_id++));

    loop.runAfter(5, std::bind(&mariadbclient::MariaDBClient::disconnect, &mariadbClient));

    loop.loop();
  }
  mysql_library_end();

  return 0;
}
