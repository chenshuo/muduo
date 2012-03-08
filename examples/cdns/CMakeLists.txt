add_library(muduo_cdns Resolver.cc)
target_link_libraries(muduo_cdns muduo_net)
target_link_libraries(muduo_cdns cares)

add_executable(cdns dns.cc)
target_link_libraries(cdns muduo_cdns)

