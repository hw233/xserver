#!/usr/bin/python
# coding: UTF-8
#怪物AI

import sys
from socket import *
import struct
import move_pb2
import cast_skill_pb2
import team_pb2
import get_one_msg

WATCH_PLAYER = {12884902632}
WATCH_MONSTER = {118111600640}

HOST='127.0.0.1'
PORT=13697
PORT=get_one_msg.get_dumpsrv_port()
ADDR=(HOST, PORT)
client=socket(AF_INET, SOCK_STREAM)
client.connect(ADDR)

last_data = ""
player_list = {}
saved_uuid = 0

while True:
    ret, last_data, player_id, msg_id, pb_data = get_one_msg.get_one_msg(client, last_data)
    if ret == -1:
        break
    if ret == 0:
        continue

#    print "%s get msgid %s" % (player_id, msg_id)
        
#    if not player_id in WATCH_PLAYER:
#        continue;

    if msg_id == 10102:
        req = move_pb2.move_notify()
        req.ParseFromString(pb_data)
        uuid = req.playerid
        if uuid / 1125899906842624 != 1:
            print "%lu move %s" % (uuid, uuid / 1125899906842624)
            continue
            
        if len(req.data) == 0:
            continue
        movedata=""
        for t1 in req.data:
            movedata = movedata + " [%.1f %.1f]" % (t1.pos_x, t1.pos_z)
#            b.append((t1.pos_x, t1.pos_z))
        print "%lu: movedata %s" % (uuid, movedata)
        

#    if msg_id == 10103:
#        req = move_pb2.sight_changed_notify()
#        req.ParseFromString(pb_data)       
#        for t1 in req.add_monster:
#            if t1.uuid in WATCH_MONSTER:
#                print "add monster %lu %u hp:%u %.1f %.1f" % (t1.uuid, t1.monsterid, t1.hp, t1.data[0].pos_x, t1.data[0].pos_z)
# 
#        for t1 in req.delete_monster:
#            if t1 in WATCH_MONSTER:
#                print "del monster %lu" % (t1)
# 
#        if msg_id == MSG_ID_TEAM_CHANGE_LEAD_NOTIFY:
#        	req = team_pb2.team_playerid()
#        	req.ParseFromString(pb_data)
#        	print req.id
