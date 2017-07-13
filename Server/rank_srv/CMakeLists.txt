FILE(GLOB SRC_LIST
  "./*.cpp"
  )

INCLUDE ("../cmake_flags")
INCLUDE_DIRECTORIES("."
#  "${PROJECT_SOURCE_DIR}/excel_data"
  "${PROJECT_SOURCE_DIR}/comm_include"
#  "${PROJECT_SOURCE_DIR}/comm_lib/include"
  "${PROJECT_SOURCE_DIR}/comm_game"
  "${PROJECT_SOURCE_DIR}/proto"    
  "${PROJECT_SOURCE_DIR}/thirdlib/libevent/include"
  "${PROJECT_SOURCE_DIR}/thirdlib/libevent"
  )

link_libraries(protobuf-c)
link_libraries(proto)
link_libraries(hiredis)
link_libraries(lua5.2)
INCLUDE ("../cmake_ldflags")

link_directories("${PROJECT_SOURCE_DIR}/thirdlib/libevent/.libs"
  )

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
add_executable  (rank_srv ${SRC_LIST})

