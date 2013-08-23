add_library(protobuf_codec codec.cc)
target_link_libraries(protobuf_codec protobuf muduo_net z)

add_custom_command(OUTPUT query.pb.cc query.pb.h
  COMMAND protoc
  ARGS --cpp_out . ${CMAKE_CURRENT_SOURCE_DIR}/query.proto -I${CMAKE_CURRENT_SOURCE_DIR}
  DEPENDS query.proto
  VERBATIM )

set_source_files_properties(query.pb.cc PROPERTIES COMPILE_FLAGS "-Wno-conversion -Wno-shadow")
include_directories(${PROJECT_BINARY_DIR})

add_library(query_proto query.pb.cc)
target_link_libraries(query_proto protobuf pthread)

add_executable(protobuf_codec_test codec_test.cc)
set_target_properties(protobuf_codec_test PROPERTIES COMPILE_FLAGS "-Wno-error=shadow")
target_link_libraries(protobuf_codec_test protobuf_codec query_proto)

add_executable(protobuf_dispatcher_lite_test dispatcher_lite_test.cc)
set_target_properties(protobuf_dispatcher_lite_test PROPERTIES COMPILE_FLAGS "-Wno-error=shadow")
target_link_libraries(protobuf_dispatcher_lite_test query_proto)

add_executable(protobuf_dispatcher_test dispatcher_test.cc)
set_target_properties(protobuf_dispatcher_test PROPERTIES COMPILE_FLAGS "-Wno-error=shadow")
target_link_libraries(protobuf_dispatcher_test query_proto)

add_executable(protobuf_server server.cc)
set_target_properties(protobuf_server PROPERTIES COMPILE_FLAGS "-Wno-error=shadow")
target_link_libraries(protobuf_server protobuf_codec query_proto)

add_executable(protobuf_client client.cc)
set_target_properties(protobuf_client PROPERTIES COMPILE_FLAGS "-Wno-error=shadow")
target_link_libraries(protobuf_client protobuf_codec query_proto)

add_custom_target(protobuf_codec_all
                  DEPENDS
                        protobuf_codec_test
                        protobuf_dispatcher_lite_test
                        protobuf_dispatcher_test
                        protobuf_server
                        protobuf_client)
