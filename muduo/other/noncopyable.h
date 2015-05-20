#ifndef MUDUO_OTHER_NONCOPYABLE_H
#define MUDUO_OTHER_NONCOPYABLE_H

namespace muduo
{

class noncopyable
{
 protected:
  noncopyable(){}
 private:
  noncopyable(const noncopyable&);
  noncopyable& operator =(const noncopyable&);
};

}

#endif // MUDUO_OTHER_COPYABLE_H
