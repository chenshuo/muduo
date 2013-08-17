add_executable(hub hub.cc codec.cc)
target_link_libraries(hub muduo_inspect)

add_library(muduo_pubsub pubsub.cc codec.cc)
target_link_libraries(muduo_pubsub muduo_net)

add_executable(pub pub.cc)
target_link_libraries(pub muduo_pubsub)

add_executable(sub sub.cc)
target_link_libraries(sub muduo_pubsub)

