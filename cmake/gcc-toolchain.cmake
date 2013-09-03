SET (CMAKE_C_COMPILER /usr/bin/gcc)
SET (CMAKE_C_FLAGS          "-std=c99 -march=native")
SET (CMAKE_CXX_COMPILER /usr/bin/g++)
SET (CMAKE_CXX_FLAGS        "-std=c++11 -march=native -fno-omit-frame-pointer -fPIC -I/usr/local/include -Wall -Wextra -Woverloaded-virtual -Wno-unused-local-typedefs --param inline-unit-growth=150 --param max-inline-insns-single=500 --param large-function-growth=600 -Wconversion -Werror")

set (CMAKE_C_FLAGS          "${CMAKE_C_FLAGS}" CACHE STRING "c flags")
set (CMAKE_CXX_FLAGS        "${CMAKE_CXX_FLAGS}" CACHE STRING "c++ flags")
SET( CMAKE_EXPORT_COMPILE_COMMANDS 1 )
set (CMAKE_EXPORT_COMPILE_COMMANDS "${CMAKE_EXPORT_COMPILE_COMMANDS}" CACHE STRING "export compile_commands.json")

IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Release CACHE STRING
      "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)
set (CMAKE_BUILD_TYPE "${CMAKE_BUILD_TYPE}" CACHE STRING "build type")
