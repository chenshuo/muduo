add_executable(fastcgi_test fastcgi.cc fastcgi_test.cc)
target_link_libraries(fastcgi_test muduo_net)

