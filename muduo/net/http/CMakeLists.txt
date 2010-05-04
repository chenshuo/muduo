set(http_SRCS
  HttpServer.cc
  HttpResponse.cc
  )

add_library(muduo_http ${http_SRCS})
target_link_libraries(muduo_http muduo_net)

add_executable(httpserver_test tests/HttpServer_test.cc)
target_link_libraries(httpserver_test muduo_http)

# add_subdirectory(tests)
