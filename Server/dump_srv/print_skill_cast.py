#!/usr/bin/python
# coding: UTF-8

import datetime
import socket
import struct
from IN import AF_INET
from ssl import SOCK_STREAM

import cast_skill_pb2


WATCH_PLAYER = {12884902939}

HOST = '127.0.0.1'
PORT = 10697
BUFSIZ = 1024
ADDR = (HOST, PORT)
client = socket(AF_INET, SOCK_STREAM)
client.connect(ADDR)

last_data = ""
player_list = {}

while True:
    data = client.recv(BUFSIZ)
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
    player_id, t1, t1, t1 = struct.unpack('=QIHH', extern_data)
    if cur_pos > data_len:
        print 'err, cur_pos[%d] > data_len[%d]' % (cur_pos, data_len)
        break
    if cur_pos == data_len:
        last_data = ''
    else:
        last_data = data[cur_pos:]
        
#    msg_format = "=IHH" + str(data_len) + 'sQIHH' 

    if player_id not in WATCH_PLAYER:
        continue

# cast skill
    if msg_id == 10200:
        req = cast_skill_pb2.skill_cast_request()
        req.ParseFromString(pb_data)
        oldtime = datetime.datetime.now()
        print oldtime.time(), ": %lu cast skill %u from pos %.1f %.1f" % \
            (player_id, req.skillid, req.cur_pos.pos_x, req.cur_pos.pos_z)

#  if msg_id == 10202:
#      req = cast_skill_pb2.skill_cast_notify()
#      req.ParseFromString(pb_data)
#      oldtime=datetime.datetime.now()
#      print oldtime.time(), ": %lu notify skill %u from pos %.1f %.1f" % \
#          (req.playerid, req.skillid, req.cur_pos.pos_x, req.cur_pos.pos_z)

    if msg_id == 10203:
        req = cast_skill_pb2.skill_hit_request()
        req.ParseFromString(pb_data)
        oldtime = datetime.datetime.now()
        
        
        
