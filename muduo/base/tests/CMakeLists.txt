add_executable(asynclogging_test AsyncLogging_test.cc)
target_link_libraries(asynclogging_test muduo_base)

add_executable(atomic_unittest Atomic_unittest.cc)
add_test(NAME atomic_unittest COMMAND atomic_unittest)

add_executable(blockingqueue_test BlockingQueue_test.cc)
target_link_libraries(blockingqueue_test muduo_base)

add_executable(blockingqueue_cpp11_test BlockingQueue_test.cc)
target_link_libraries(blockingqueue_cpp11_test muduo_base_cpp11)
set_target_properties(blockingqueue_cpp11_test PROPERTIES COMPILE_FLAGS "-std=c++0x")

add_executable(blockingqueue_bench BlockingQueue_bench.cc)
target_link_libraries(blockingqueue_bench muduo_base)

add_executable(boundedblockingqueue_test BoundedBlockingQueue_test.cc)
target_link_libraries(boundedblockingqueue_test muduo_base)

add_executable(date_unittest Date_unittest.cc)
target_link_libraries(date_unittest muduo_base)
add_test(NAME date_unittest COMMAND date_unittest)

add_executable(exception_test Exception_test.cc)
target_link_libraries(exception_test muduo_base)
add_test(NAME exception_test COMMAND exception_test)

add_executable(fileutil_test FileUtil_test.cc)
target_link_libraries(fileutil_test muduo_base)
add_test(NAME fileutil_test COMMAND fileutil_test)

add_executable(fork_test Fork_test.cc)
target_link_libraries(fork_test muduo_base)

if(ZLIB_FOUND)
  add_executable(gzipfile_test GzipFile_test.cc)
  target_link_libraries(gzipfile_test muduo_base_cpp11 z)
  set_target_properties(gzipfile_test PROPERTIES COMPILE_FLAGS "-std=c++0x")
  add_test(NAME gzipfile_test COMMAND gzipfile_test)
endif()

add_executable(logfile_test LogFile_test.cc)
target_link_libraries(logfile_test muduo_base)

add_executable(logging_test Logging_test.cc)
target_link_libraries(logging_test muduo_base)

add_executable(logstream_bench LogStream_bench.cc)
target_link_libraries(logstream_bench muduo_base)

if(BOOSTTEST_LIBRARY)
add_executable(logstream_test LogStream_test.cc)
target_link_libraries(logstream_test muduo_base boost_unit_test_framework)
add_test(NAME logstream_test COMMAND logstream_test)
endif()

add_executable(mutex_test Mutex_test.cc)
target_link_libraries(mutex_test muduo_base)

add_executable(processinfo_test ProcessInfo_test.cc)
target_link_libraries(processinfo_test muduo_base)

add_executable(singleton_test Singleton_test.cc)
target_link_libraries(singleton_test muduo_base)

add_executable(singleton_threadlocal_test SingletonThreadLocal_test.cc)
target_link_libraries(singleton_threadlocal_test muduo_base)

add_executable(thread_bench Thread_bench.cc)
target_link_libraries(thread_bench muduo_base)

add_executable(thread_test Thread_test.cc)
target_link_libraries(thread_test muduo_base)

add_executable(threadlocal_test ThreadLocal_test.cc)
target_link_libraries(threadlocal_test muduo_base)

add_executable(threadlocalsingleton_test ThreadLocalSingleton_test.cc)
target_link_libraries(threadlocalsingleton_test muduo_base)

add_executable(threadpool_test ThreadPool_test.cc)
target_link_libraries(threadpool_test muduo_base)

add_executable(timestamp_unittest Timestamp_unittest.cc)
target_link_libraries(timestamp_unittest muduo_base)
add_test(NAME timestamp_unittest COMMAND timestamp_unittest)

add_executable(timezone_unittest TimeZone_unittest.cc)
target_link_libraries(timezone_unittest muduo_base)
add_test(NAME timezone_unittest COMMAND timezone_unittest)

