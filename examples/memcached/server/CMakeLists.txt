if(BOOSTPO_LIBRARY)
  add_executable(memcached-debug Item.cc MemcacheServer.cc Session.cc server.cc)
  target_link_libraries(memcached-debug muduo_net boost_program_options)
endif()

