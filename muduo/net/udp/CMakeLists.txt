set(udp_SRCS
  UdpClient.cc
  UdpMessage.cc
  UdpServer.cc
  )

add_library(muduo_net_udp ${udp_SRCS})
target_link_libraries(muduo_net_udp muduo_net)

install(TARGETS muduo_net_udp DESTINATION lib)
set(HEADERS
  UdpClient.h
  UdpMessage.h
  UdpServer.h
  )
install(FILES ${HEADERS} DESTINATION include/muduo/net/udp)
