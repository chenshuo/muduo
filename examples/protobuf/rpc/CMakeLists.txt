add_custom_command(OUTPUT sudoku.pb.cc sudoku.pb.h
  COMMAND protoc
  ARGS --cpp_out . ${CMAKE_CURRENT_SOURCE_DIR}/sudoku.proto -I${CMAKE_CURRENT_SOURCE_DIR}
  DEPENDS sudoku.proto)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error=conversion -Wno-extra -Wno-shadow")
include_directories(${PROJECT_BINARY_DIR})

add_library(sudoku_proto sudoku.pb.cc)
target_link_libraries(sudoku_proto protobuf pthread)

#TODO set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error=conversion")

add_executable(protobuf_rpc_sudoku_client client.cc)
target_link_libraries(protobuf_rpc_sudoku_client sudoku_proto muduo_protorpc)

add_executable(protobuf_rpc_sudoku_server server.cc)
target_link_libraries(protobuf_rpc_sudoku_server sudoku_proto muduo_protorpc)

add_custom_target(protobuf_rpc_all
                  DEPENDS
                        muduo_protorpc
                        protobuf_rpc_sudoku_client
                        protobuf_rpc_sudoku_server
                        protobuf_rpc_echo_client
                        protobuf_rpc_echo_server
                        )
