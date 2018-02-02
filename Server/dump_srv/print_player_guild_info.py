#!/usr/bin/python
# coding: UTF-8

import sys
from socket import *
import raid_pb2
import move_pb2
import cast_skill_pb2
import move_direct_pb2
import role_pb2
import datetime
import get_one_msg
import get_uuid_type

WATCH_PLAYER = {}
WATCH_UUID = {1125901403839415}

HOST='127.0.0.1'
PORT=13697
PORT=get_one_msg.get_dumpsrv_port()
ADDR=(HOST, PORT)
client=socket(AF_INET, SOCK_STREAM)
client.connect(ADDR)

last_data = ""
player_list = {}


while True:
    ret, last_data, player_id, msg_id, pb_data = get_one_msg.get_one_msg(client, last_data)
    if ret == -1:
        break
    if ret == 0:
        continue

    #心跳
    if msg_id == 10009:
        continue

    if len(WATCH_PLAYER) != 0 and not player_id in WATCH_PLAYER:
        continue
#    if not player_id in WATCH_PLAYER:
#        continue

#MSG_ID_SIGHT_PLAYER_CHANGE_NOTIFY 10124  //视野玩家信息变更通知 sight_player_info_change_notify
    if msg_id == 10124:
	req = move_pb2.sight_player_info_change_notify()
	req.ParseFromString(pb_data)
	oldtime=datetime.datetime.now()

	print oldtime.time(), "%lu: player[%lu] guild_id[%u] name[%s] office[%u]" % (player_id, req.player_id, req.guild_id, req.guild_name, req.guild_office)

        
