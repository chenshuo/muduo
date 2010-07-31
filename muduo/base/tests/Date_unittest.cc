#include <muduo/base/Date.h>
#include <stdio.h>

using muduo::Date;

void passByConstReference(const Date& x)
{
  printf("%s\n", x.toString().c_str());
}

void passByValue(Date x)
{
  printf("%s\n", x.toString().c_str());
}

int main()
{
  Date someDay(1982, 4, 3);
  printf("%s\n", someDay.toString().c_str());
  passByValue(someDay);
  passByConstReference(someDay);
}

