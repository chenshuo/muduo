#include <muduo/base/Exception.h>
#include <stdio.h>

class Bar
{
 public:
  void test()
  {
    throw muduo::Exception("oops");
  }
};

void foo()
{
  Bar b;
  b.test();
}

int main()
{
  try
  {
    foo();
  }
  catch (const muduo::Exception& ex)
  {
    printf("reason: %s\n", ex.what());
    printf("stack trace: %s\n", ex.stackTrace());
  }
}
