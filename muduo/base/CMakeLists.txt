set(base_SRCS 
  Logging.cc
  Timestamp.cc
  Thread.cc
  )

add_library(muduo_base ${base_SRCS})

add_subdirectory(tests)
