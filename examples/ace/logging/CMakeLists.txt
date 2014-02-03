add_custom_command(OUTPUT logrecord.pb.cc logrecord.pb.h
  COMMAND protoc
  ARGS --cpp_out . ${CMAKE_CURRENT_SOURCE_DIR}/logrecord.proto -I${CMAKE_CURRENT_SOURCE_DIR}
  DEPENDS logrecord.proto
  VERBATIM )

set_source_files_properties(logrecord.pb.cc PROPERTIES COMPILE_FLAGS "-Wno-conversion -Wno-shadow")
include_directories(${PROJECT_BINARY_DIR})

add_library(ace_logging_proto logrecord.pb.cc)
target_link_libraries(ace_logging_proto protobuf pthread)

add_executable(ace_logging_client client.cc)
set_target_properties(ace_logging_client PROPERTIES COMPILE_FLAGS "-Wno-error=shadow")
target_link_libraries(ace_logging_client muduo_protobuf_codec ace_logging_proto)

add_executable(ace_logging_server server.cc)
set_target_properties(ace_logging_server PROPERTIES COMPILE_FLAGS "-Wno-error=shadow")
target_link_libraries(ace_logging_server muduo_protobuf_codec ace_logging_proto)
