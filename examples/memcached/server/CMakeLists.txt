if(BOOSTPO_LIBRARY)
  add_executable(memcached-debug Item.cc MemcacheServer.cc Session.cc server.cc)
  target_link_libraries(memcached-debug muduo_net boost_program_options)
endif()

add_executable(memcached-footprint Item.cc MemcacheServer.cc Session.cc footprint_test.cc)
target_link_libraries(memcached-footprint muduo_net muduo_inspect)

find_path(TCMALLOC_INCLUDE_DIR google/heap-profiler.h)
find_library(TCMALLOC_LIBRARY NAMES tcmalloc)

if(TCMALLOC_INCLUDE_DIR AND TCMALLOC_LIBRARY)
  message(STATUS "found tcmalloc")
  set_target_properties(memcached-footprint PROPERTIES COMPILE_FLAGS "-DHAVE_TCMALLOC")
  target_link_libraries(memcached-footprint tcmalloc)
  target_link_libraries(memcached-debug tcmalloc)
else()
  message(STATUS "not found tcmalloc")
endif()

