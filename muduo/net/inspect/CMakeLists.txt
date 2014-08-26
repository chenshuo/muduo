set(inspect_SRCS
  Inspector.cc
  PerformanceInspector.cc
  ProcessInspector.cc
  SystemInspector.cc
  )

add_library(muduo_inspect ${inspect_SRCS})
target_link_libraries(muduo_inspect muduo_http)

if(TCMALLOC_INCLUDE_DIR AND TCMALLOC_LIBRARY)
  set_target_properties(muduo_inspect PROPERTIES COMPILE_FLAGS "-DHAVE_TCMALLOC")
  target_link_libraries(muduo_inspect tcmalloc_and_profiler)
endif()

install(TARGETS muduo_inspect DESTINATION lib)
set(HEADERS
  Inspector.h
  )
install(FILES ${HEADERS} DESTINATION include/muduo/net/inspect)

if(NOT CMAKE_BUILD_NO_EXAMPLES)
add_executable(inspector_test tests/Inspector_test.cc)
target_link_libraries(inspector_test muduo_inspect)
endif()

