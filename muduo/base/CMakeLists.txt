set(base_SRCS 
  Date.cc
  Logging.cc
  ProcessInfo.cc
  Timestamp.cc
  TimeZone.cc
  Thread.cc
  )

add_library(muduo_base ${base_SRCS})

add_subdirectory(tests)
