set(net_SRCS
  Acceptor.cc
  Channel.cc
  DefaultPoller.cc
  EPollPoller.cc
  EventLoop.cc
  InetAddress.cc
  Poller.cc
  PollPoller.cc
  Socket.cc
  SocketsOps.cc
  TcpServer.cc
  Timer.cc
  TimerQueue.cc
  )

add_library(muduo_net ${net_SRCS})
target_link_libraries(muduo_net muduo_base)

add_subdirectory(tests)
