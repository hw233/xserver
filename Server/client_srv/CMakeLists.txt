FILE(GLOB SRC_LIST
  "./*.cpp"
  )

add_definitions(-g -O0 -Wall -Werror -DDEBUG -fPIC -fstack-protector-all)
INCLUDE_DIRECTORIES("."
  "${PROJECT_SOURCE_DIR}/excel_data"
  "${PROJECT_SOURCE_DIR}/comm_include"
#  "${PROJECT_SOURCE_DIR}/comm_lib/include"
  "${PROJECT_SOURCE_DIR}/comm_game"
  "${PROJECT_SOURCE_DIR}/proto"    
  "${PROJECT_SOURCE_DIR}/thirdlib/libevent/include"
  "${PROJECT_SOURCE_DIR}/thirdlib/libevent"
  )

link_libraries(game)
link_libraries(event)
link_libraries(protobuf-c)
link_libraries(log4c)
link_libraries(proto)
link_libraries(rt)
link_libraries(curl)
link_libraries(json-c)

link_directories("${PROJECT_SOURCE_DIR}/thirdlib/libevent/.libs"
  )

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
add_executable  (client_srv ${SRC_LIST})

