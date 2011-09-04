add_subdirectory(codec)
add_subdirectory(rpc)
add_subdirectory(rpcbench)
if(PROTOBUF_PLUGIN)
  add_subdirectory(rpc2)
  add_subdirectory(rpcbench2)
else()
  add_subdirectory(rpc2 EXCLUDE_FROM_ALL)
  add_subdirectory(rpcbench2 EXCLUDE_FROM_ALL)
endif()


