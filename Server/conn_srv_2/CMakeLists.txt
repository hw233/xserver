FILE(GLOB_RECURSE SRC_LIST
  "./*.cpp"
  "player.pb-c.c"
  )


INCLUDE ("../cmake_flags")
INCLUDE_DIRECTORIES(
  "${PROJECT_SOURCE_DIR}/comm_include"
#  "${PROJECT_SOURCE_DIR}/comm_lib/include"
  "${PROJECT_SOURCE_DIR}/comm_game"
  "${PROJECT_SOURCE_DIR}/proto"    
  "${PROJECT_SOURCE_DIR}/thirdlib/libevent/include"
  "${PROJECT_SOURCE_DIR}/thirdlib/libevent"
  )

add_custom_target(conn2_proto
  COMMAND protoc-c --c_out=. player.proto)

link_libraries(protobuf-c)
link_libraries(proto)
link_libraries(crypto)
INCLUDE ("../cmake_ldflags")

link_directories("${PROJECT_SOURCE_DIR}/thirdlib/libevent/.libs"
  )

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
add_executable  (conn_srv_2 ${SRC_LIST})

#> link_libraries(event)
#> link_libraries(log4c)
#> add_executable  (game_srv ${SRC_LIST})
