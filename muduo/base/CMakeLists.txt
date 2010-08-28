set(base_SRCS 
  CountDownLatch.cc
  Date.cc
  Exception.cc
  Logging.cc
  ProcessInfo.cc
  Timestamp.cc
  Thread.cc
  ThreadPool.cc
  )

add_library(muduo_base ${base_SRCS})

add_subdirectory(tests)
