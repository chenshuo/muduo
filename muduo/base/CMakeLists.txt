set(base_SRCS 
  UtcTime.cc)

add_library(muduo_base ${base_SRCS})

add_subdirectory(tests)
