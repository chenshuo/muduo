set(http_SRCS
  HttpServer.cc
  )

add_library(muduo_http ${http_SRCS})
target_link_libraries(muduo_http muduo_net)

# add_subdirectory(tests)
