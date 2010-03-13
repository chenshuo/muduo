set(net_SRCS 
  Channel.cc
  DefaultPoller.cc
  EPollPoller.cc
  EventLoop.cc
  Poller.cc
  PollPoller.cc
  Socket.cc
  Timer.cc
  TimerQueue.cc
  )

add_library(muduo_net ${net_SRCS})
target_link_libraries(muduo_net muduo_base)

add_subdirectory(tests)
