add_library(muduo_paxos paxos.cc)
target_link_libraries(muduo_paxos muduo_inspect)

add_library(muduo_lobby lobby.cc)
target_link_libraries(muduo_lobby muduo_inspect)

