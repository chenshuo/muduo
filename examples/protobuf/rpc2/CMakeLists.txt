add_custom_command(OUTPUT sudoku.pb.cc sudoku.pb.h
  COMMAND protoc
  ARGS --plugin=${PROJECT_BINARY_DIR}/bin/protoc-gen-rpc --rpc_out . ${CMAKE_CURRENT_SOURCE_DIR}/sudoku.proto -I${CMAKE_CURRENT_SOURCE_DIR}
  COMMAND sed
  ARGS -i "'s/#include <google\\/protobuf\\/service.h>/#include <muduo\\/net\\/protorpc2\\/service.h>/'" sudoku.pb.h
  MAIN_DEPENDENCY sudoku.proto
  DEPENDS protoc-gen-rpc)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error=conversion -Wno-extra")
include_directories(${PROJECT_BINARY_DIR})

add_library(sudoku_proto2 sudoku.pb.cc)
target_link_libraries(sudoku_proto2 protobuf pthread)

#TODO set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error=conversion")

add_executable(protobuf_rpc_sudoku_client2 client.cc)
target_link_libraries(protobuf_rpc_sudoku_client2 sudoku_proto2 muduo_protorpc2)

add_executable(protobuf_rpc_sudoku_server2 server.cc)
target_link_libraries(protobuf_rpc_sudoku_server2 sudoku_proto2 muduo_protorpc2)

add_custom_target(protobuf_rpc2_all
                  DEPENDS
                        protoc-gen-rpc
                        muduo_protorpc2
                        protobuf_rpc_sudoku_client2
                        protobuf_rpc_sudoku_server2
                        protobuf_rpc_echo_client2
                        protobuf_rpc_echo_server2
                        )
