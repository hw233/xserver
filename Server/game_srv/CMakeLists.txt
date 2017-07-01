FILE(GLOB SRC_LIST
  "./*.cpp"
  )

INCLUDE ("../cmake_flags")
INCLUDE_DIRECTORIES("."
  "${PROJECT_SOURCE_DIR}/excel_data"
  "${PROJECT_SOURCE_DIR}/comm_include"
  "${PROJECT_SOURCE_DIR}/comm_lib/include"
  "${PROJECT_SOURCE_DIR}/comm_game"
  "${PROJECT_SOURCE_DIR}/thirdlib/libevent/include"
  "${PROJECT_SOURCE_DIR}/thirdlib/libevent"
  )

link_libraries(game)
link_libraries(event)
link_libraries(log4c)
link_libraries(rt)
link_libraries(dl)
link_libraries(python2.7)

link_directories("${PROJECT_SOURCE_DIR}/thirdlib/libevent/.libs"
  )

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
add_executable  (game_srv ${SRC_LIST})

#> link_libraries(event)
#> link_libraries(log4c)
#> add_executable  (game_srv ${SRC_LIST})
