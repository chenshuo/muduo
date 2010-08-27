// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef MUDUO_BASE_THREADLOCALSINGLETON_H
#define MUDUO_BASE_THREADLOCALSINGLETON_H

#include <boost/noncopyable.hpp>

namespace muduo
{

template<typename T>
class ThreadLocalSingleton : boost::noncopyable
{
 public:

  static T& instance()
  {
    if (!t_value_)
    {
      t_value_ = new T();
    }
    return *t_value_;
  }

  // no way to auto delete it
  static void destroy()
  {
    delete t_value_;
    t_value_ = 0;
  }

 private:

  static __thread T* t_value_;
};

template<typename T>
__thread T* ThreadLocalSingleton<T>::t_value_ = 0;

}
#endif
