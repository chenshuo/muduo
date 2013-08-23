add_custom_command(OUTPUT rpc.pb.cc rpc.pb.h
  COMMAND protoc
  ARGS --cpp_out . ${CMAKE_CURRENT_SOURCE_DIR}/rpc.proto -I${CMAKE_CURRENT_SOURCE_DIR}
  DEPENDS rpc.proto
  VERBATIM )

set_source_files_properties(rpc.pb.cc PROPERTIES COMPILE_FLAGS "-Wno-conversion")
include_directories(${PROJECT_BINARY_DIR})

add_library(muduo_protorpc_wire rpc.pb.cc RpcCodec.cc)
set_target_properties(muduo_protorpc_wire PROPERTIES COMPILE_FLAGS "-Wno-error=shadow")

add_library(muduo_protorpc_wire_cpp11 rpc.pb.cc RpcCodec.cc)
set_target_properties(muduo_protorpc_wire_cpp11 PROPERTIES COMPILE_FLAGS "-Wno-error=shadow")

add_library(muduo_protorpc RpcChannel.cc RpcServer.cc)
set_target_properties(muduo_protorpc PROPERTIES COMPILE_FLAGS "-Wno-error=shadow")
target_link_libraries(muduo_protorpc muduo_protorpc_wire muduo_net protobuf z)

install(TARGETS muduo_protorpc_wire muduo_protorpc DESTINATION lib)
install(TARGETS muduo_protorpc_wire_cpp11 DESTINATION lib)

set(HEADERS
  RpcCodec.h
  RpcChannel.h
  RpcServer.h
  rpc.proto
  rpcservice.proto
  ${PROJECT_BINARY_DIR}/muduo/net/protorpc/rpc.pb.h
  )
install(FILES ${HEADERS} DESTINATION include/muduo/net/protorpc)

