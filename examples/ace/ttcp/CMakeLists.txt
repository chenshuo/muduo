if(BOOSTPO_LIBRARY)
  add_executable(ttcp_blocking ttcp_blocking.cc common.cc main.cc)
  target_link_libraries(ttcp_blocking muduo_base boost_program_options)
  set_target_properties(ttcp_blocking PROPERTIES COMPILE_FLAGS "-Wno-error=old-style-cast -Wno-error=conversion")

  add_executable(ttcp_muduo ttcp.cc common.cc main.cc)
  target_link_libraries(ttcp_muduo muduo_net boost_program_options)

  if(BOOSTSYSTEM_LIBRARY)
    add_executable(ttcp_asio_sync ttcp_asio_sync.cc common.cc main.cc)
    target_link_libraries(ttcp_asio_sync muduo_base boost_program_options boost_system)

    add_executable(ttcp_asio_async ttcp_asio_async.cc common.cc main.cc)
    target_link_libraries(ttcp_asio_async muduo_base boost_program_options boost_system)
  endif()
endif()

