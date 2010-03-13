set(base_SRCS 
  Thread.cc
  UtcTime.cc
  )

add_library(muduo_base ${base_SRCS})

add_subdirectory(tests)
