add_custom_command(OUTPUT resolver.pb.cc resolver.pb.h
  COMMAND protoc
  ARGS --cpp_out . ${CMAKE_CURRENT_SOURCE_DIR}/resolver.proto -I${CMAKE_CURRENT_SOURCE_DIR}
  DEPENDS resolver.proto)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error=conversion -Wno-extra -Wno-shadow")
include_directories(${PROJECT_BINARY_DIR})

add_library(resolver_proto resolver.pb.cc)
target_link_libraries(resolver_proto protobuf pthread)

#TODO set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error=conversion")

add_executable(protobuf_rpc_resolver_client client.cc)
target_link_libraries(protobuf_rpc_resolver_client resolver_proto muduo_protorpc)

add_executable(protobuf_rpc_resolver_server server.cc)
target_link_libraries(protobuf_rpc_resolver_server resolver_proto muduo_protorpc muduo_cdns)
