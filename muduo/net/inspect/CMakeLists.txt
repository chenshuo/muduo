set(inspect_SRCS
  Inspector.cc
  ProcessInspector.cc
  )

add_library(muduo_inspect ${inspect_SRCS})
target_link_libraries(muduo_inspect muduo_http)

install(TARGETS muduo_inspect DESTINATION lib)
file(GLOB HEADERS "*.h")
install(FILES ${HEADERS} DESTINATION include/muduo/net/inspect)

if(NOT CMAKE_BUILD_NO_EXAMPLES)
add_executable(inspector_test tests/Inspector_test.cc)
target_link_libraries(inspector_test muduo_inspect)
endif()

