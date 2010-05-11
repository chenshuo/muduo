set(inspect_SRCS
  Inspector.cc
  ProcessInspector.cc
  )

add_library(muduo_inspect ${inspect_SRCS})
target_link_libraries(muduo_inspect muduo_http)

add_executable(inspector_test tests/Inspector_test.cc)
target_link_libraries(inspector_test muduo_inspect)

