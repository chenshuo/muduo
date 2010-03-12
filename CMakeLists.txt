cmake_minimum_required(VERSION 2.6)

project(muduo CXX)

include_directories(${PROJECT_SOURCE_DIR})
add_subdirectory(muduo/base)
add_subdirectory(muduo/net)
