add_executable(netty_echo_client client.cc)
target_link_libraries(netty_echo_client muduo_net)

add_executable(netty_echo_server server.cc)
target_link_libraries(netty_echo_server muduo_net)

add_executable(netty_echo_server2 server2.cc)
target_link_libraries(netty_echo_server2 muduo_net)

