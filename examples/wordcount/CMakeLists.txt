if(Boost_FOUND)
  add_executable(wordcount_hasher hasher.cc)
  target_link_libraries(wordcount_hasher muduo_net)
endif()

add_executable(wordcount_receiver receiver.cc)
target_link_libraries(wordcount_receiver muduo_net)
