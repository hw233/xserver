FILE(GLOB_RECURSE SRC_LIST
  "./*.cpp"
  )

INCLUDE ("../cmake_flags")

add_definitions(-Wno-pointer-arith)

INCLUDE_DIRECTORIES(
  "include"
  "${PROJECT_SOURCE_DIR}/comm_include"
  "${PROJECT_SOURCE_DIR}/comm_game"  
  "${PROJECT_SOURCE_DIR}/excel_data"
  "${PROJECT_SOURCE_DIR}/proto"      
  "${PROJECT_SOURCE_DIR}/thirdlib/libevent/include"
  "${PROJECT_SOURCE_DIR}/thirdlib/libevent"
  )

link_libraries(mysqlclient)
link_libraries(hiredis)
link_libraries(pthread)
link_libraries(z)
link_libraries(m)
link_libraries(dl)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
add_library  (game ${SRC_LIST})

#> link_libraries(game)
#> link_libraries(event)
#> link_libraries(log4c)
#> add_executable  (game_srv ${SRC_LIST})
