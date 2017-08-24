if(Boost_FOUND)
  add_executable(procmon procmon.cc plot.cc)
  target_link_libraries(procmon muduo_http gd)
endif()

add_executable(plot_test plot_test.cc plot.cc)
target_link_libraries(plot_test muduo_base gd)

add_executable(dummyload dummyload.cc)
target_link_libraries(dummyload muduo_net)
