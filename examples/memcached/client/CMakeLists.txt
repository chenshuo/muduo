add_executable(memcached-bench bench.cc)
target_link_libraries(memcached-bench muduo_net)
