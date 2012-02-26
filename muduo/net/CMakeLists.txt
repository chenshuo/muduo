set(net_SRCS
  Acceptor.cc
  Buffer.cc
  Channel.cc
  Connector.cc
  EventLoop.cc
  EventLoopThread.cc
  EventLoopThreadPool.cc
  InetAddress.cc
  Poller.cc
  poller/DefaultPoller.cc
  poller/EPollPoller.cc
  poller/PollPoller.cc
  Socket.cc
  SocketsOps.cc
  TcpClient.cc
  TcpConnection.cc
  TcpServer.cc
  Timer.cc
  TimerQueue.cc
  )

add_library(muduo_net ${net_SRCS})
target_link_libraries(muduo_net muduo_base)

install(TARGETS muduo_net DESTINATION lib)
set(HEADERS
  Buffer.h
  Callbacks.h
  Endian.h
  EventLoop.h
  EventLoopThread.h
  InetAddress.h
  TcpClient.h
  TcpConnection.h
  TcpServer.h
  TimerId.h
  )
install(FILES ${HEADERS} DESTINATION include/muduo/net)

add_subdirectory(http)
add_subdirectory(inspect)
add_subdirectory(tests)

if(PROTOBUF_FOUND)
  add_subdirectory(protorpc)
  if(PROTOBUF_PLUGIN)
    add_subdirectory(protorpc2)
  else()
    add_subdirectory(protorpc2 EXCLUDE_FROM_ALL)
  endif()
else()
  add_subdirectory(protorpc EXCLUDE_FROM_ALL)
  add_subdirectory(protorpc2 EXCLUDE_FROM_ALL)
endif()
