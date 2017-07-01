FILE(GLOB_RECURSE SRC_LIST
  "./*.c"
  )

INCLUDE ("../cmake_flags")
link_libraries(protobuf-c)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
add_library  (proto SHARED ${SRC_LIST})

