set(MUDUO_THRIFT_SRCS
    ThriftConnection.cc
    ThriftServer.cc
    )
add_library(muduo_thrift ${MUDUO_THRIFT_SRCS})
target_link_libraries(muduo_thrift muduo_net thrift)

if(THRIFT_COMPILER AND THRIFT_INCLUDE_DIR AND THRIFT_LIBRARY)
  include_directories(${CMAKE_CURRENT_SOURCE_DIR})
  add_subdirectory(tests)
endif()
