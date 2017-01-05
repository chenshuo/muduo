set(tls_SRCS
  TlsConfig.cc
  )

add_library(muduo_tls ${tls_SRCS})
target_link_libraries(muduo_tls muduo_net ${tls} ${ssl} ${crypto})

install(TARGETS muduo_tls DESTINATION lib)

set(HEADERS
  TlsConfig.h
  )
install(FILES ${HEADERS} DESTINATION include/muduo/net/tls)

add_subdirectory(tests)
