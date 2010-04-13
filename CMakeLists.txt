cmake_minimum_required(VERSION 2.6)

project(muduo CXX)

set(CXX_FLAGS
 -g
 -DVALGRIND
 -Wall
 -Wextra
 # -Werror
 -Wconversion
 -Wunused-parameter
 -Wold-style-cast
 -Wpointer-arith
 -Wshadow
 -Wwrite-strings
 # -MMD
 # -std=c++0x
 -rdynamic
 -lpthread
 )
string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")

set(CMAKE_CXX_FLAGS_DEBUG "-O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O2") # no NDEBUG in Release, keep asserting.
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

include_directories(${PROJECT_SOURCE_DIR})

add_subdirectory(muduo/base)
add_subdirectory(muduo/net)

add_subdirectory(examples)

