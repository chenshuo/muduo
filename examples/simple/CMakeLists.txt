add_executable(simple_discard discard/discard.cc discard/main.cc)
target_link_libraries(simple_discard muduo_net)

add_executable(simple_daytime daytime/daytime.cc daytime/main.cc)
target_link_libraries(simple_daytime muduo_net)

add_executable(simple_time time/time.cc time/main.cc)
target_link_libraries(simple_time muduo_net)

add_executable(simple_echo echo/echo.cc echo/main.cc)
target_link_libraries(simple_echo muduo_net)

add_executable(simple_allinone
  discard/discard.cc 
  daytime/daytime.cc 
  time/time.cc 
  echo/echo.cc 
  allinone/allinone.cc)
target_link_libraries(simple_allinone muduo_net)

add_executable(simple_timeclient timeclient/timeclient.cc)
target_link_libraries(simple_timeclient muduo_net)

