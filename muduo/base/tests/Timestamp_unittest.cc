#include <muduo/base/Timestamp.h>
#include <stdio.h>

using muduo::Timestamp;

void passByConstReference(const Timestamp& x)
{
  printf("%s\n", x.toString().c_str());
}

void passByValue(Timestamp x)
{
  printf("%s\n", x.toString().c_str());
}

int main()
{
  Timestamp now(Timestamp::now());
  printf("%s\n", now.toString().c_str());
  passByValue(now);
  passByConstReference(now);
}

