add_executable(idleconnection_echo echo.cc main.cc)
target_link_libraries(idleconnection_echo muduo_net)
