#include "MariaDBClient.h"

#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>

using namespace muduo;
using namespace muduo::net;

static uint64_t g_seqid = 0;

void updateCallback(mariadbclient::MariaDBClient* c, uint64_t id)
{
  LOG_INFO << "seq id: " << id << "\terrorNo: " << c->errorNo() << "\terrorStr: " << c->errorStr();
}

void queryCallback(mariadbclient::MariaDBClient* c, MYSQL_RES* result, uint64_t id)
{
  LOG_INFO << "seq id: " << id << "\terrorNo: " << c->errorNo() << "\terrorStr: " << c->errorStr();

  uint32_t numFields = ::mysql_num_fields(result);
  MYSQL_ROW row;
  while ((row = ::mysql_fetch_row(result)) != NULL)
  {
    uint64_t* lengths = ::mysql_fetch_lengths(result);
    for (uint32_t i = 0; i < numFields; ++i)
    {
      printf("[%.*s] ", static_cast<uint32_t>(lengths[i]), row[i] ? row[i] : "NULL");
    }
    printf("\n");
  }

  ::mysql_free_result(result);
}

void connectCallback(mariadbclient::MariaDBClient* c)
{
  if (c->errorNo() == 0)
  {
    LOG_INFO << "Connected... " << "\terrorNo: " << c->errorNo() << "\terrorStr: " << c->errorStr();

    string sql0("DROP TABLE muduo_user");
    string sql1("CREATE TABLE muduo_user ("
                    "id INT(11) NOT NULL AUTO_INCREMENT,"
                    "nick VARCHAR(64) NOT NULL,"
                    "PRIMARY KEY (id)"
                ")");
    string sql2("SELECT id, nick "
                  "FROM muduo_user");
    string sql3("INSERT INTO muduo_user (id, nick)"
                "VALUES (1, 'ChenShuo')");
    string sql4("INSERT INTO muduo_user (nick)"
                "VALUES ('Jack')");
    string sql5("INSERT INTO muduo_user (nick)"
                "VALUES ('Lucy')");
    string sql6("UPDATE muduo_user "
                   "SET nick = 'Tom' "
                 "WHERE id = 2");
    string sql7("SELECT id, nick "
                  "FROM muduo_user");
    string sql8("DELETE FROM muduo_user "
                 "WHERE id > 1");
    string sql9("SELECT id, nick "
                  "FROM muduo_user");

    c->executeUpdate(sql0, std::bind(&updateCallback, _1, g_seqid++));
    c->executeUpdate(sql1, std::bind(&updateCallback, _1, g_seqid++));
    c->executeQuery(sql2, std::bind(&queryCallback, _1, _2, g_seqid++));
    c->executeUpdate(sql3, std::bind(&updateCallback, _1, g_seqid++));
    c->executeUpdate(sql4); g_seqid++;
    c->executeUpdate(sql5); g_seqid++;
    c->executeUpdate(sql6, std::bind(&updateCallback, _1, g_seqid++));
    c->executeQuery(sql7, std::bind(&queryCallback, _1, _2, g_seqid++));
    c->executeUpdate(sql8, std::bind(&updateCallback, _1, g_seqid++));
    c->executeQuery(sql9, std::bind(&queryCallback, _1, _2, g_seqid++));
  }
  else
  {
    LOG_ERROR << "connectCallback Error: " << "\terrorNo: " << c->errorNo() << "\terrorStr: " << c->errorStr();
  }
}

void disconnectCallback(mariadbclient::MariaDBClient* c, EventLoop* loop)
{
  if (c->errorNo() == 0)
  {
    LOG_INFO << "Disconnected... " << "\terrorNo: " << c->errorNo() << "\terrorStr: " << c->errorStr();
  }
  else
  {
    LOG_ERROR << "disconnectCallback Error: " << "\terrorNo: " << c->errorNo() << "\terrorStr: " << c->errorStr();
  }

  loop->quit();
}

int main(int argc, char* argv[])
{
  int err = mysql_library_init(0, NULL, NULL);
  if (err != 0)
  {
    LOG_FATAL << "mysql_library_init() returns error: " << err;
  }

  EventLoop loop;
  mariadbclient::MariaDBClient mariadbClient(&loop, InetAddress("127.0.0.1", 3306), "root", "123456", "test");
  mariadbClient.setConnectCallback(std::bind(&connectCallback, _1));
  mariadbClient.setDisconnectCallback(std::bind(&disconnectCallback, _1, &loop));
  mariadbClient.connect();
  loop.runAfter(5, std::bind(&mariadbclient::MariaDBClient::disconnect, &mariadbClient));

  loop.loop();

  mysql_library_end();

  return 0;
}
