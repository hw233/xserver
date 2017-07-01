#!/usr/bin/python
# coding: UTF-8
#释放技能

import sys
from socket import *
import struct
import move_pb2
import move_direct_pb2
import cast_skill_pb2

WATCH_PLAYER = {12884902769}

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

#施法请求
    if msg_id == 10200:
        req = cast_skill_pb2.skill_cast_request()
        req.ParseFromString(pb_data)
        print "cast skillid = %d" % (req.skillid)
#施法广播
    if msg_id == 10202:
        req = cast_skill_pb2.skill_cast_notify()
        req.ParseFromString(pb_data)
        print "cast notify: id[%lu] skill[%lu]" % (req.playerid, req.skillid)
#命中请求        
    if msg_id == 10203:
        req = cast_skill_pb2.skill_hit_request()
        req.ParseFromString(pb_data)        
        for t1 in req.target_playerid:
            print "hit target %lu" % (t1)
#命中广播
    if msg_id == 10205:
        req = cast_skill_pb2.skill_hit_notify()
        req.ParseFromString(pb_data)        
        for t1 in req.target_player:
            print "hit notify id[%lu] effect[%d] hpdelta[%d] hp[%d]" % (t1.playerid, t1.effect, t1.hp_delta, t1.cur_hp)
            for t2 in t1.add_buff:
                print "hit[%lu] notify add buff %d" % (t1.playerid, t2)
        
