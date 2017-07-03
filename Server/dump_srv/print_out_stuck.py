#!/usr/bin/python
# coding: UTF-8

import sys
from socket import *
import struct
import comm_message_pb2
import setting_pb2
import time

WATCH_PLAYER = {12884902859}

HOST='127.0.0.1'
PORT=11697
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
    if msg_id == 10425:
        res = setting_pb2.TransferOutStuckAnswer()
        res.ParseFromString(pb_data)       
        print "player:%lu, result:%u, ts:%u, now:%u" % (player_id, res.result, res.CDTimestamp, time.time())
    elif msg_id == 10426:
        nty = setting_pb2.TransferOutStuckInfoNotify()
        nty.ParseFromString(pb_data)
        print "player:%lu, ts:%u, now:%u" % (player_id, nty.CDTimestamp, time.time())


    
