#!/usr/bin/python
# coding: UTF-8

import sys
from socket import *
import move_pb2
import move_direct_pb2
import get_one_msg
import datetime

WATCH_PLAYER = {12884903020, 12884903024}

HOST='127.0.0.1'
PORT=10697
PORT=get_one_msg.get_dumpsrv_port()
ADDR=(HOST, PORT)
client=socket(AF_INET, SOCK_STREAM)
client.connect(ADDR)

last_data = ""
#player_list = {12884902631, 12884902635}

while True:
    ret, last_data, player_id, msg_id, pb_data = get_one_msg.get_one_msg(client, last_data)
    if ret == -1:
        break
    if ret == 0:
        continue

#    if player_id not in WATCH_PLAYER:
#        continue;
# 移动停止
    if msg_id == 10108:
        req = move_direct_pb2.move_stop_request()
        req.ParseFromString(pb_data)
        oldtime = datetime.datetime.now()                
        print oldtime.time(), " move stop[%.1f][%.1f]" % (req.cur_pos.pos_x, req.cur_pos.pos_z)

#移动开始        
    if msg_id == 10105:
        req = move_direct_pb2.move_start_request()
        req.ParseFromString(pb_data)
        oldtime = datetime.datetime.now()                
        print oldtime.time(), " move start[%.1f][%.1f]" % (req.cur_pos.pos_x, req.cur_pos.pos_z)

    if msg_id == 10106 or msg_id == 10109 or msg_id == 10101:
        req = move_pb2.move_answer()
        req.ParseFromString(pb_data)
        oldtime = datetime.datetime.now()                
        if req.result == 0:
            print oldtime.time(), " move success len[%d]" % (len(pb_data))
        else:
            print oldtime.time(), " move failed, pos = [%.1f][%.1f] len[%d]" % (req.pos.pos_x, req.pos.pos_z, len(pb_data))
            
