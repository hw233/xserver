#!/bin/bash
open_gm=`cat server_info.ini | grep open_gm_cmd | sed 's/^open_gm_cmd=//g'`
server_id=`cat server_info.ini | grep game_srv_id | sed 's/^game_srv_id=//g'`
client_port=`cat server_info.ini | grep conn_srv_client_port | sed 's/^conn_srv_client_port=//g'`
shm_addr=`cat server_info.ini | grep game_srv_player_key | sed 's/^game_srv_player_key=//g'`
player_num=`cat server_info.ini | grep game_srv_player_num | sed 's/^game_srv_player_num=//g'`
./generate_config.sh ${server_id} ${client_port} ${shm_addr} ${player_num} > server_info.ini
sed  -i "s/open_gm_cmd=[01]/open_gm_cmd=${open_gm}/g" server_info.ini
