add_library(muduo_cdns Resolver.cc)
target_link_libraries(muduo_cdns muduo_net)
target_link_libraries(muduo_cdns cares)

install(TARGETS muduo_cdns DESTINATION lib)
install(FILES Resolver.h DESTINATION include/muduo/cdns)

add_executable(cdns dns.cc)
target_link_libraries(cdns muduo_cdns)

