add_custom_command(OUTPUT rpc.pb.cc rpc.pb.h
  COMMAND protoc
  ARGS --cpp_out . ${CMAKE_CURRENT_SOURCE_DIR}/rpc.proto -I${CMAKE_CURRENT_SOURCE_DIR}
  DEPENDS rpc.proto
  VERBATIM )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error=conversion -Wno-extra")
include_directories(${PROJECT_BINARY_DIR})

add_library(muduo_protorpc rpc.pb.cc RpcCodec.cc RpcChannel.cc RpcServer.cc)
target_link_libraries(muduo_protorpc muduo_net protobuf z)

install(TARGETS muduo_protorpc DESTINATION lib)
set(HEADERS
  RpcCodec.h
  RpcChannel.h
  RpcServer.h
  rpc.proto
  rpcservice.proto
  ${PROJECT_BINARY_DIR}/muduo/net/protorpc/rpc.pb.h
  )
install(FILES ${HEADERS} DESTINATION include/muduo/net/protorpc)

