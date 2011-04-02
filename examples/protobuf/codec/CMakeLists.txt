add_library(protobuf_codec codec.cc)
target_link_libraries(protobuf_codec protobuf muduo_net)

add_custom_command(OUTPUT query.pb.cc query.pb.h
  COMMAND protoc
  ARGS --cpp_out . ${CMAKE_CURRENT_SOURCE_DIR}/query.proto -I${CMAKE_CURRENT_SOURCE_DIR}
  DEPENDS query.proto
  VERBATIM )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error=conversion")
include_directories(${PROJECT_BINARY_DIR})

add_library(query_proto query.pb.cc)
target_link_libraries(query_proto protobuf)

#TODO set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error=conversion")

add_executable(protobuf_codec_test codec_test.cc)
target_link_libraries(protobuf_codec_test protobuf_codec query_proto)

