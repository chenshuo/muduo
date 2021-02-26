add_library(protobuf_codec codec.cc)
target_link_libraries(protobuf_codec protobuf muduo_net z)

add_custom_command(OUTPUT query.pb.cc query.pb.h
  COMMAND protoc
  ARGS --cpp_out . ${CMAKE_CURRENT_SOURCE_DIR}/query.proto -I${CMAKE_CURRENT_SOURCE_DIR}
  DEPENDS query.proto
  VERBATIM )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error=conversion -Wno-extra")
include_directories(${PROJECT_BINARY_DIR})

add_library(query_proto query.pb.cc)
target_link_libraries(query_proto protobuf pthread)

#TODO set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error=conversion")

add_executable(protobuf_codec_test codec_test.cc)
target_link_libraries(protobuf_codec_test protobuf_codec query_proto)

add_executable(protobuf_dispatcher_lite_test dispatcher_lite_test.cc)
target_link_libraries(protobuf_dispatcher_lite_test query_proto)

add_executable(protobuf_dispatcher_test dispatcher_test.cc)
target_link_libraries(protobuf_dispatcher_test query_proto)

add_executable(protobuf_server server.cc)
target_link_libraries(protobuf_server protobuf_codec query_proto)

add_executable(protobuf_client client.cc)
target_link_libraries(protobuf_client protobuf_codec query_proto)

add_custom_target(protobuf_codec_all
                  DEPENDS
                        protobuf_codec_test
                        protobuf_dispatcher_lite_test
                        protobuf_dispatcher_test
                        protobuf_server
                        protobuf_client)
