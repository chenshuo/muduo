add_library(muduo_protobuf_codec ProtobufCodecLite.cc)
set_target_properties(muduo_protobuf_codec PROPERTIES COMPILE_FLAGS "-Wno-error=shadow")
target_link_libraries(muduo_protobuf_codec muduo_net protobuf z)

#add_library(muduo_protobuf_codec_cpp11 ProtobufCodecLite.cc)
#set_target_properties(muduo_protobuf_codec_cpp11 PROPERTIES COMPILE_FLAGS "-std=c++0x -Wno-error=shadow")
#target_link_libraries(muduo_protobuf_codec_cpp11 muduo_net_cpp11 protobuf z)


install(TARGETS muduo_protobuf_codec DESTINATION lib)
#install(TARGETS muduo_protobuf_codec_cpp11 DESTINATION lib)

file(GLOB HEADERS "*.h")
install(FILES ${HEADERS} DESTINATION include/muduo/net/protobuf)

