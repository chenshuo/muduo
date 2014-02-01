if(BOOSTPO_LIBRARY)
  add_executable(memcached_debug Item.cc MemcacheServer.cc Session.cc server.cc)
  target_link_libraries(memcached_debug muduo_net muduo_inspect boost_program_options)
endif()

add_executable(memcached_footprint Item.cc MemcacheServer.cc Session.cc footprint_test.cc)
target_link_libraries(memcached_footprint muduo_net muduo_inspect)

if(TCMALLOC_INCLUDE_DIR AND TCMALLOC_LIBRARY)
  set_target_properties(memcached_footprint PROPERTIES COMPILE_FLAGS "-DHAVE_TCMALLOC")
  if(BOOSTPO_LIBRARY)
    set_target_properties(memcached_debug PROPERTIES COMPILE_FLAGS "-DHAVE_TCMALLOC")
  endif()
endif()

