#include <muduo/base/GzipFile.h>

int main()
{
  const char* filename = "/tmp/gzipfile_test.gz";
  ::unlink(filename);
  const char data[] = "123456789012345678901234567890123456789012345678901234567890\n";
  {
  muduo::GzipFile writer = muduo::GzipFile::openForAppend(filename);
  if (writer.valid())
  {
    printf("tell %ld\n", writer.tell());
    printf("wrote %d\n", writer.write(data));
    printf("tell %ld\n", writer.tell());
  }
  }

  {
  printf("testing reader\n");
  muduo::GzipFile reader = muduo::GzipFile::openForRead(filename);
  if (reader.valid())
  {
    char buf[256];
    printf("tell %ld\n", reader.tell());
    int nr = reader.read(buf, sizeof buf);
    printf("read %d\n", nr);
    if (nr >= 0)
    {
      buf[nr] = '\0';
      printf("data %s", buf);
    }
    printf("tell %ld\n", reader.tell());
    if (strncmp(buf, data, strlen(data)) != 0)
    {
      printf("failed!!!\n");
      abort();
    }
    else
    {
      printf("PASSED\n");
    }
  }
  }

  {
  muduo::GzipFile writer = muduo::GzipFile::openForWriteExclusive(filename);
  if (writer.valid() || errno != EEXIST)
  {
    printf("FAILED\n");
  }
  }
}
