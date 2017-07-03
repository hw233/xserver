#!/usr/bin/python
# coding: UTF-8

import sys
from socket import *
import struct
import move_pb2
import move_direct_pb2
import datetime

WATCH_PLAYER = {12884903020, 12884903024}

HOST='127.0.0.1'
PORT=10697
BUFSIZ=1024
ADDR=(HOST, PORT)
client=socket(AF_INET, SOCK_STREAM)
client.connect(ADDR)

last_data = ""
#player_list = {12884902631, 12884902635}

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

    if not player_id in WATCH_PLAYER:
        continue;
# 移动停止
    if msg_id == 10108:
        req = move_direct_pb2.move_stop_request()
        req.ParseFromString(pb_data)
        oldtime=datetime.datetime.now()                
        print oldtime.time() , " move stop[%.1f][%.1f]" % (req.cur_pos.pos_x, req.cur_pos.pos_z)        

#移动开始        
    if msg_id == 10105:
        req = move_direct_pb2.move_start_request()
        req.ParseFromString(pb_data)
        oldtime=datetime.datetime.now()                
        print oldtime.time() ," move start[%.1f][%.1f]" % (req.cur_pos.pos_x, req.cur_pos.pos_z)
