add_executable(protoc-gen-rpc gen.cc)
target_link_libraries(protoc-gen-rpc protoc pthread)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error=conversion -Wno-extra")
include_directories(${PROJECT_BINARY_DIR})

add_library(muduo_protorpc2 ../protorpc/rpc.pb.cc ../protorpc/RpcCodec.cc RpcChannel.cc RpcServer.cc)
target_link_libraries(muduo_protorpc2 muduo_net protobuf)
set_source_files_properties(../protorpc/rpc.pb.cc PROPERTIES GENERATED yes)
add_dependencies(muduo_protorpc2 protoc-gen-rpc)

install(TARGETS protoc-gen-rpc DESTINATION bin)
install(TARGETS muduo_protorpc2 DESTINATION lib)
set(HEADERS
  RpcCodec.h
  RpcChannel.h
  RpcServer.h
  )
install(FILES ${HEADERS} DESTINATION include/muduo/net/protorpc2)
