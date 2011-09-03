add_custom_command(OUTPUT echo.pb.cc echo.pb.h
  COMMAND protoc
  ARGS --plugin=${PROJECT_BINARY_DIR}/bin/protoc-gen-rpc --rpc_out . ${CMAKE_CURRENT_SOURCE_DIR}/echo.proto -I${CMAKE_CURRENT_SOURCE_DIR}
  COMMAND sed
  ARGS -i "'s/#include <google\\/protobuf\\/service.h>/#include <muduo\\/net\\/protorpc\\/service.h>/'" echo.pb.h
  MAIN_DEPENDENCY echo.proto
  DEPENDS protoc-gen-rpc)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error=conversion -Wno-extra")
include_directories(${PROJECT_BINARY_DIR})

add_library(echo_proto echo.pb.cc)
target_link_libraries(echo_proto protobuf pthread)

#TODO set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error=conversion")

add_executable(protobuf_rpc_echo_client client.cc)
target_link_libraries(protobuf_rpc_echo_client echo_proto muduo_protorpc)

add_executable(protobuf_rpc_echo_server server.cc)
target_link_libraries(protobuf_rpc_echo_server echo_proto muduo_protorpc)
