#!/usr/bin/python
# coding: UTF-8

import sys
from socket import *
import struct
import move_pb2
import cast_skill_pb2
import move_direct_pb2
import datetime
import get_one_msg

WATCH_PLAYER = {12884902921, 12884902818}
WATCH_MONSTER = set()

HOST='127.0.0.1'
PORT=10697
BUFSIZ=1024
ADDR=(HOST, PORT)
client=socket(AF_INET, SOCK_STREAM)
client.connect(ADDR)

last_data = ""
player_list = {}
MONSTER_ID = 151004002
monster_uuid = 0

while True:
    ret, last_data, player_id, msg_id, pb_data = get_one_msg.get_one_msg(client, last_data)    
    if ret == -1:
        break
    if ret == 0:
        continue
    oldtime=datetime.datetime.now()        
        
#视野变化    
    if msg_id == 10103:
        req = move_pb2.sight_changed_notify()
        req.ParseFromString(pb_data)
        for t1 in req.add_monster:
            if t1.monsterid != MONSTER_ID:
#                print oldtime.time(), "add other monster %s %.1f %.1f" % (t1.monsterid, t1.data[0].pos_x, t1.data[0].pos_z)
                continue
            monster_uuid = t1.uuid
            print oldtime.time(), "add monster %lx %.1f %.1f speed[%.1f]" % (t1.uuid, t1.data[0].pos_x, t1.data[0].pos_z, t1.speed)

        for t1 in req.delete_monster:
            if t1 != monster_uuid:
                continue
            print oldtime.time(), "del monster %lx" % (t1)

    if msg_id == 10206:
        req = cast_skill_pb2.skill_hit_immediate_notify()
        req.ParseFromString(pb_data)
        if req.playerid == monster_uuid:
            print oldtime.time(), "[%lx] attack [%lx] attack_pos[%.1f][%.1f]" % (req.playerid,
                req.target_player.playerid, req.attack_pos.pos_x, req.attack_pos.pos_z);
        if req.target_player.playerid == monster_uuid:
            print oldtime.time(), "[%lx] attack [%lx] beattack_pos[%.1f][%.1f]" % (req.playerid,
                req.target_player.playerid, req.target_player.target_pos.pos_x, req.target_player.target_pos.pos_z)

    if msg_id == 10102:
        req = move_pb2.move_notify()
        req.ParseFromString(pb_data)
        if req.playerid == monster_uuid:
            movedata=""
            for t1 in req.data:
                movedata = movedata + " [%.1f %.1f]" % (t1.pos_x, t1.pos_z)
            print oldtime.time(), "%lx: movedata %s" % (monster_uuid, movedata)
            
