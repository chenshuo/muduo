#include <muduo/base/Logging.h>

#include <stdio.h>

int g_total;
FILE* g_file;

void dummyOutput(const char* msg, int len)
{
  g_total += len;
  if (g_file)
  {
    fwrite(msg, 1, len, g_file);
  }
}

void bench()
{
  muduo::Logger::setOutput(dummyOutput);
  muduo::Timestamp start(muduo::Timestamp::now());
  g_total = 0;

  int n = 1000*1000;
  for (int i = 0; i < n; ++i)
  {
    LOG_INFO << "Hello 0123456789" << " abcdefghijklmnopqrstuvwxyz" << i;
  }
  muduo::Timestamp end(muduo::Timestamp::now());
  double seconds = timeDifference(end, start);
  printf("%f seconds, %d bytes, %.2f msg/s, %.2f MiB/s\n",
         seconds, g_total, n / seconds, g_total / seconds);
}

int main()
{
  getppid(); // for ltrace and strace
  LOG_TRACE << "trace";
  LOG_DEBUG << "debug";
  LOG_INFO << "Hello";
  LOG_WARN << "World";
  LOG_ERROR << "Error";
  LOG_INFO << sizeof(muduo::Logger);
  LOG_INFO << sizeof(muduo::LogStream);
  LOG_INFO << sizeof(muduo::Fmt);
  LOG_INFO << sizeof(muduo::LogStream::Buffer);

  bench();

  char buffer[64*1024];

  g_file = fopen("/dev/null", "w");
  setbuffer(g_file, buffer, sizeof buffer);
  bench();
  fclose(g_file);

  g_file = fopen("/tmp/log", "w");
  setbuffer(g_file, buffer, sizeof buffer);
  bench();
  fclose(g_file);

}
