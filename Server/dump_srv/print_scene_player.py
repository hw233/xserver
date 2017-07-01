#!/usr/bin/python
# coding: UTF-8

import sys
from socket import *
import struct
import move_pb2
import cast_skill_pb2
import move_direct_pb2
import team_pb2
import datetime
import get_one_msg

WATCH_PLAYER = {4294968631}


HOST='127.0.0.1'
PORT=13697
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
#    data_len = data_len - 8 - 16
#    msg_format = "=IHH" + str(data_len) + 'sQIHH' 
#    msg_len, msg_id, seq, pb_data, player_id, t1, t1, t1 = struct.unpack(msg_format, data)

#    print "read msg:", msg_id	

    if not player_id in WATCH_PLAYER:
        continue;

#视野变化    
    if msg_id == 10103:
        req = move_pb2.sight_changed_notify()
        req.ParseFromString(pb_data)
        oldtime=datetime.datetime.now()        
        for t1 in req.add_player:
            print oldtime.time(), ": %lu add player %lx hp:%u %.1f %.1f" % (player_id, t1.playerid, t1.hp, t1.data[0].pos_x, t1.data[0].pos_z)

        for t1 in req.delete_player:
            print oldtime.time(), ": %lu del player %lx" % (player_id, t1)
#ready
    if msg_id == 10104:
        oldtime=datetime.datetime.now()        
        print oldtime.time(), ": %lu ready now" % player_id

#死亡离开视野            
    if msg_id == 10205:
        req = cast_skill_pb2.skill_hit_notify()
        req.ParseFromString(pb_data)
        oldtime=datetime.datetime.now()
        print_head = " %lu " % player_id
        movedata = ""
        for t1 in req.target_player:
            if t1.cur_hp <= 0:
                movedata = movedata + " killed[%lx]" % (t1.playerid)
        if len(movedata) > 0:
            print oldtime.time() , print_head, movedata

	if msg_id == MSG_ID_TEAM_CHANGE_LEAD_NOTIFY:
		req = team_pb2.team_playerid()
		req.ParseFromString(pb_data)
		print req.id
    
