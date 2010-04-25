set(inspect_SRCS
  Inspector.cc
  )

add_library(muduo_inspect ${inspect_SRCS})
target_link_libraries(muduo_inspect muduo_net)

# add_subdirectory(tests)
