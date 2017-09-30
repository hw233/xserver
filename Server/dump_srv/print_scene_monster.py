#!/usr/bin/python
# coding: UTF-8

import sys
from socket import *
import struct
import move_pb2
import cast_skill_pb2
import move_direct_pb2
import datetime

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

while True:
    data=client.recv(BUFSIZ)
    data = last_data + data
    data_len = len(data)
    if data_len == 0:
        break
    if data_len < 8:
        last_data = data
        continue
    msg_head = data[0:8]
    cur_pos = 8    
    msg_len, msg_id, seq = struct.unpack('=IHH', msg_head)

    if msg_len > data_len:
        last_data = data
        continue

    pb_len = msg_len - 8 - 16
    pb_data = data[cur_pos:cur_pos+pb_len]
    cur_pos = cur_pos + pb_len
    extern_data = data[cur_pos:cur_pos+16]
    cur_pos = cur_pos + 16
    player_id,t1,t1,t1 = struct.unpack('=QIHH', extern_data)
    if cur_pos > data_len:
        print 'err, cur_pos[%d] > data_len[%d]' %(cur_pos, data_len)
        break
    if cur_pos == data_len:
        last_data = ''
    else:
        last_data = data[cur_pos:]
        
#    data_len = data_len - 8 - 16
#    msg_format = "=IHH" + str(data_len) + 'sQIHH' 
#    msg_len, msg_id, seq, pb_data, player_id, t1, t1, t1 = struct.unpack(msg_format, data)

#    if not player_id in WATCH_PLAYER:
#        continue;

#视野变化    
    if msg_id == 10103:
        req = move_pb2.sight_changed_notify()
        req.ParseFromString(pb_data)
        oldtime = datetime.datetime.now()        
        for t1 in req.add_monster:
            if t1.uuid in WATCH_MONSTER:
                print 'err add ', t1.uuid
            WATCH_MONSTER.add(t1.uuid)
            
#            if t1.uuid in WATCH_MONSTER:
            print oldtime.time(), ": %lu %d add monster %lx %u hp:%u %.1f %.1f" % (player_id, len(WATCH_MONSTER), t1.uuid, t1.monsterid, t1.hp, t1.data[0].pos_x, t1.data[0].pos_z)

        for t1 in req.delete_monster:
            if not t1 in WATCH_MONSTER:
                print 'err delete ', t1            
            WATCH_MONSTER.discard(t1)
#            if t1 in WATCH_MONSTER:
            print oldtime.time(), ": %lu %d del monster %lx" % (player_id, len(WATCH_MONSTER), t1)

#死亡离开视野            
    if msg_id == 10205:
        req = cast_skill_pb2.skill_hit_notify()
        req.ParseFromString(pb_data)
        oldtime = datetime.datetime.now()
        print_head = " %lu " % player_id
        movedata = ""
        for t1 in req.target_player:
            if t1.cur_hp <= 0:
                if t1.playerid not in WATCH_MONSTER:
                    print 'err delete ', t1.playerid
                WATCH_MONSTER.discard(t1.playerid)
                movedata = movedata + " killed[%lx]" % (t1.playerid)
        if len(movedata) > 0:
            print oldtime.time(), print_head, len(WATCH_MONSTER), movedata

    
