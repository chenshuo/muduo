add_custom_command(OUTPUT sudoku.pb.cc sudoku.pb.h
  COMMAND protoc
  ARGS --cpp_out . ${CMAKE_CURRENT_SOURCE_DIR}/sudoku.proto -I${CMAKE_CURRENT_SOURCE_DIR}
  DEPENDS sudoku.proto)

set_source_files_properties(sudoku.pb.cc PROPERTIES COMPILE_FLAGS "-Wno-conversion -Wno-shadow")
include_directories(${PROJECT_BINARY_DIR})

add_library(sudoku_proto sudoku.pb.cc)
target_link_libraries(sudoku_proto protobuf pthread)

add_executable(protobuf_rpc_sudoku_client client.cc)
set_target_properties(protobuf_rpc_sudoku_client PROPERTIES COMPILE_FLAGS "-Wno-error=shadow")
target_link_libraries(protobuf_rpc_sudoku_client sudoku_proto muduo_protorpc)

add_executable(protobuf_rpc_sudoku_server server.cc)
set_target_properties(protobuf_rpc_sudoku_server PROPERTIES COMPILE_FLAGS "-Wno-error=shadow")
target_link_libraries(protobuf_rpc_sudoku_server sudoku_proto muduo_protorpc)

add_custom_target(protobuf_rpc_all
                  DEPENDS
                        muduo_protorpc
                        protobuf_rpc_balancer
                        protobuf_rpc_balancer_raw
                        protobuf_rpc_echo_client
                        protobuf_rpc_echo_server
                        protobuf_rpc_resolver_client
                        protobuf_rpc_resolver_server
                        protobuf_rpc_sudoku_client
                        protobuf_rpc_sudoku_server
                        )
