cmake_minimum_required(VERSION 2.6)

project(muduo CXX)

set(CMAKE_CXX_FLAGS
  "-g -Wall -Wextra -Werror -Wconversion -Wold-style-cast -Wpointer-arith -Wshadow -Wno-unused-parameter -Wwrite-strings -rdynamic"
  )
include_directories(${PROJECT_SOURCE_DIR})
add_subdirectory(muduo/base)
add_subdirectory(muduo/net)
