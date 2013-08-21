if(BOOSTPO_LIBRARY)
  add_executable(memcached-debug Item.cc MemcacheServer.cc Session.cc server.cc)
  target_link_libraries(memcached-debug muduo_net muduo_inspect boost_program_options)
endif()

add_executable(memcached-footprint Item.cc MemcacheServer.cc Session.cc footprint_test.cc)
target_link_libraries(memcached-footprint muduo_net muduo_inspect)

if(TCMALLOC_INCLUDE_DIR AND TCMALLOC_LIBRARY)
  set_target_properties(memcached-footprint PROPERTIES COMPILE_FLAGS "-DHAVE_TCMALLOC")
  if(BOOSTPO_LIBRARY)
    set_target_properties(memcached-debug PROPERTIES COMPILE_FLAGS "-DHAVE_TCMALLOC")
  endif()
endif()

