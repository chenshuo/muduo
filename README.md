*README*

# DOC #
---
Muduo is based on boost which is a large monst lib. And muduo also use cmake to build & link. This branch just has two targets:
- 1 use C++11 instead of boost;
- 2 use makefile instead of cmake.

# Regulations of using C++11 instead of boost #
---
- 1 boost::shared_ptr -> std::shared_ptr;
- 2 boost::weak_ptr -> std::weak_ptr;
- 3 boost::scoped_ptr -> std::unique_ptr;
	Pls attention that unique_ptr does not have unique_ptr::release method，its inner class object will live as long as the program.
- 4 get_pointer(unique_ptr) -> unique_ptr.get()；
- 5 boost::ptr_vector<T> -> std::vector<std::unique_ptr<T>>；
- 6 boost::ptr_vector<T>(size) -\-> std::vector<std::unique_ptr<T>>(size);
    The above symbol "-\->" means "not equals to". boost::ptr_vector<T>::ptr_vector(size). The "ptr_vector(size_type to_reserve)" just constructs an empty vector with a buffer of size least to_reserve. While std::vector<std::unique_ptr<T>>(v_size) Constructs a container with a buffer of size "v_size" and fill each element of this container with object of type std::unique_ptr<T>(code example:muduo/base/ests/BlockingQueue_bench.cc和muduo/base/ests/BlockingQueue_test.cc).
- 7 boost::ptr_vector<T>::pop_back ->  std::vector<std::unique_ptr<T>>::back + std::vector<std::unique_ptr<T>>::pop_back;
    Pls attention that the return value of boost::ptr_vector<T>::pop_back is its last element while the return value of std::vector::pop_back is void.
- 8 new T + boost::ptr_vector<T>::push_back -> std::vector<std::unique_ptr<T>>::push_bck(new T);
    Pls attention that std::unique_ptr does not support copy construct and assignment operator. So I complete std::vector<T>::push_back task just in one step to create a new class object and assign this new object to a std::unique_ptr object(code example:EventLoopThreadPool.ccmuduo/net/EventLoopThreadPool.cc line45-line50).
- 9 boost::bind-> std::bind;
	*Pls attention that the function that std::bind use should be C-style function of C++ class static funtion(code example:muduo/net/TcpConnection.cc line110 & line131*).
- 10 boost::any-> cdiggins::any(muduo/other/any.h)
- 11 boost::noncopyable -> muduo::noncopyable(muduo/base/noncopyable.h)
- 12 add offsetof macro in muduo/net/InetAddress.cc

# Change Log #
---
- 1 create this project 2015-05-20.
- 2 sync with github.com/chenshuo/muduo master on 2016-10-16 which lastest version is v1.0.8.
- 3 sync with https://github.com/chenshuo/muduo/tree/cpp11 on 2017-09-14 which lastest version is v1.0.9.
- 4 add contrib/hiredis on 2017-10-17.