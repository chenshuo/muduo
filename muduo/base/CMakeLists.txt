set(base_SRCS
  CountDownLatch.cc
  Date.cc
  Exception.cc
  FileUtil.cc
  Logging.cc
  ProcessInfo.cc
  Timestamp.cc
  Thread.cc
  ThreadPool.cc
  )

add_library(muduo_base ${base_SRCS})
target_link_libraries(muduo_base pthread)

install(TARGETS muduo_base DESTINATION lib)
file(GLOB HEADERS "*.h")
install(FILES ${HEADERS} DESTINATION include/muduo/base)

if(NOT CMAKE_BUILD_NO_EXAMPLES)
  add_subdirectory(tests)
endif()
