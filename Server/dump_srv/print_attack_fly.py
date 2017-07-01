#!/usr/bin/python
# coding: UTF-8
#怪物AI

import sys
from socket import *
import struct
import move_pb2
import move_direct_pb2
import cast_skill_pb2
import datetime

WATCH_PLAYER = {12884902964}

HOST='127.0.0.1'
PORT=10697
BUFSIZ=1024
ADDR=(HOST, PORT)
client=socket(AF_INET, SOCK_STREAM)
client.connect(ADDR)

last_data = ""
player_list = {}
saved_uuid = 0

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
        
#    if not player_id in WATCH_PLAYER:
#        continue;

#skill_cast_request
    if msg_id == 10200:
        req = cast_skill_pb2.skill_cast_request()
        req.ParseFromString(pb_data)
        oldtime=datetime.datetime.now()        
        print oldtime.time() , "%.1f %.1f" % (req.cur_pos.pos_x, req.cur_pos.pos_z)

#skill_hit_notify
    if msg_id == 10205:
        req = cast_skill_pb2.skill_hit_notify()
        req.ParseFromString(pb_data)
        uuid = req.playerid

        if len(req.target_player) == 0:
            continue

#skill_hit_immediate_notify        
    if msg_id == 10206:
        req = cast_skill_pb2.skill_hit_immediate_notify()
        req.ParseFromString(pb_data)
        logdata = ""
        t1 = req.target_player
        for t2 in t1.add_buff:
            logdata = logdata + " buff[%u]" % t2
        logdata = logdata + " move to [%.1f][%.1f]" % (t1.target_pos.pos_x, t1.target_pos.pos_z)
        oldtime=datetime.datetime.now()
        print oldtime.time() , "怪打人", logdata

#
    if msg_id == 10108:
        req = move_direct_pb2.move_stop_request()
        req.ParseFromString(pb_data)
        oldtime=datetime.datetime.now()
        print oldtime.time() , "stopmove %.1f %.1f" % (req.cur_pos.pos_x, req.cur_pos.pos_z)


#move_notify        
    if msg_id == 10102:
        req = move_pb2.move_notify()
        req.ParseFromString(pb_data)
        uuid = req.playerid
        if len(req.data) == 0:
            continue
        movedata=""
        for t1 in req.data:
            movedata = movedata + " [%.1f %.1f]" % (t1.pos_x, t1.pos_z)

        oldtime=datetime.datetime.now()
#        print oldtime.time() , "[%lu]: movedata %s" % (uuid, movedata)
