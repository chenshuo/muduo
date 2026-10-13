set(LIB_FUZZING_ENGINE "$ENV{LIB_FUZZING_ENGINE}" CACHE STRING
    "Fuzzing engine library and linker flags")
if(NOT LIB_FUZZING_ENGINE)
  message(FATAL_ERROR "MUDUO_BUILD_FUZZERS requires LIB_FUZZING_ENGINE")
endif()
separate_arguments(FUZZING_ENGINE_FLAGS UNIX_COMMAND "${LIB_FUZZING_ENGINE}")

add_executable(muduo_http_fuzzer muduo_http_fuzzer.cpp)
set_target_properties(muduo_http_fuzzer PROPERTIES COMPILE_FLAGS
    "-Wno-old-style-cast")
target_link_libraries(muduo_http_fuzzer ${FUZZING_ENGINE_FLAGS} muduo_http)