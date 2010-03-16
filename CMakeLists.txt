cmake_minimum_required(VERSION 2.6)

project(muduo CXX)

set(CXX_FLAGS
 -g
 -Wall
 -Wextra
 # -Werror
 -Wconversion
 -Wunused-parameter
 -Wold-style-cast
 -MMD
 -Wpointer-arith
 -Wshadow
 -Wwrite-strings
 -rdynamic
 -lpthread
 )
string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")

set(CMAKE_CXX_FLAGS_DEBUG "-O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O2") # no NDEBUG in Release, keep asserting.

include_directories(${PROJECT_SOURCE_DIR})
add_subdirectory(muduo/base)
add_subdirectory(muduo/net)

