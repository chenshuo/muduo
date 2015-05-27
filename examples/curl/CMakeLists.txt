add_library(muduo_curl Curl.cc)
target_link_libraries(muduo_curl muduo_net)
target_link_libraries(muduo_curl curl)

install(TARGETS muduo_curl DESTINATION lib)
install(FILES Curl.h DESTINATION include/muduo/curl)

add_executable(mcurl mcurl.cc)
target_link_libraries(mcurl muduo_curl)

add_executable(curl_download download.cc)
target_link_libraries(curl_download muduo_curl)

