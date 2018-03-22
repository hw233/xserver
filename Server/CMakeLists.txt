CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
project(XGame)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "./cmake/")
find_package(Lua 5.2  REQUIRED)

add_custom_target(protoup
  COMMAND ${PROJECT_SOURCE_DIR}/../Proto/create_server_file.sh
  COMMAND make -C proto)

add_custom_target(proto_file
  COMMAND ${PROJECT_SOURCE_DIR}/proto/proto2c.sh)

add_custom_target(svnup
  COMMAND make clean
  COMMAND svn up ${PROJECT_SOURCE_DIR}/../Proto
  COMMAND cd ${PROJECT_SOURCE_DIR}/lua_data && svn up .
  COMMAND svn up ${PROJECT_SOURCE_DIR}
  COMMAND cd ${PROJECT_SOURCE_DIR}/excel_data && make excel_data
  COMMAND make -j protoup)

option(MEMERR
    "check mem overflow" ON)

#add_definitions(-DUSE_AISRV)

add_subdirectory(proto)
add_subdirectory(comm_lib)
add_subdirectory(conn_srv)
add_subdirectory(conn_srv_2)
add_subdirectory(game_srv)
add_subdirectory(game_srv/so_game_srv)

#add_subdirectory(raid_srv)
#add_subdirectory(raid_srv/so_game_srv)

#add_subdirectory(ai_srv)
#add_subdirectory(ai_srv/so_ai_srv)

add_subdirectory(db_srv)
add_subdirectory(login_srv)
add_subdirectory(dump_srv)
add_subdirectory(client_srv)
add_subdirectory(item_srv)
add_subdirectory(friend_srv)
add_subdirectory(mail_srv)
add_subdirectory(guild_srv)
add_subdirectory(rank_srv)
add_subdirectory(doufachang_srv)
add_subdirectory(trade_srv)
add_subdirectory(activity_srv)

