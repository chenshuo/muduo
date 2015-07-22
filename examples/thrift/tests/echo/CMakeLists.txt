include_directories(gen-cpp)
set(ECHO_THRIFT echo.thrift)
execute_process(COMMAND ${THRIFT_COMPILER} --gen cpp ${ECHO_THRIFT}
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
execute_process(COMMAND ${THRIFT_COMPILER} --gen py ${ECHO_THRIFT}
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
set(ECHO_THRIFT_SRCS
    gen-cpp/echo_constants.cpp
    gen-cpp/echo_types.cpp
    gen-cpp/Echo.cpp
    )
set(ECHO_SRCS
    EchoServer.cc
    )
add_executable(muduo_thrift_echo ${ECHO_SRCS} ${ECHO_THRIFT_SRCS})
target_link_libraries(muduo_thrift_echo muduo_thrift)
