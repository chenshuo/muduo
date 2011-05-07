add_subdirectory(simple)
add_subdirectory(roundtrip)
add_subdirectory(pingpong)
add_subdirectory(protobuf/codec EXCLUDE_FROM_ALL)
add_subdirectory(idleconnection)
add_subdirectory(maxconnection)
add_subdirectory(multiplexer)
add_subdirectory(hub)
add_subdirectory(filetransfer)
add_subdirectory(socks4a)

add_subdirectory(asio/tutorial)
add_subdirectory(asio/chat)

add_subdirectory(netty/discard)
add_subdirectory(netty/echo)
add_subdirectory(netty/uptime)

add_subdirectory(twisted/finger)
