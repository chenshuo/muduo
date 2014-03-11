add_executable(protobuf_rpc_balancer balancer.cc)
set_target_properties(protobuf_rpc_balancer PROPERTIES COMPILE_FLAGS "-Wno-error=shadow")
target_link_libraries(protobuf_rpc_balancer muduo_protorpc)
include_directories(${PROJECT_BINARY_DIR})
