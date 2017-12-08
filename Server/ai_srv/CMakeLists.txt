FILE(GLOB SRC_LIST
  "./*.cpp"
  )

INCLUDE ("../cmake_flags")
add_definitions(-D__AI_SRV__)
INCLUDE_DIRECTORIES("."
  "${PROJECT_SOURCE_DIR}/excel_data"
  "${PROJECT_SOURCE_DIR}/comm_include"
  "${PROJECT_SOURCE_DIR}/comm_lib/include"
  "${PROJECT_SOURCE_DIR}/comm_game"
  "${PROJECT_SOURCE_DIR}/proto"  
  "${PROJECT_SOURCE_DIR}/thirdlib/libevent/include"
  "${PROJECT_SOURCE_DIR}/thirdlib/libevent"
  )

link_libraries(dl)
#link_libraries(python2.7)
INCLUDE ("../cmake_ldflags")

link_directories("${PROJECT_SOURCE_DIR}/thirdlib/libevent/.libs"
  )

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
add_executable  (ai_srv ${SRC_LIST})

#> link_libraries(event)
#> link_libraries(log4c)
#> add_executable  (game_srv ${SRC_LIST})
