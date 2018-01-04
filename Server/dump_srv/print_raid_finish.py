#!/usr/bin/python
# coding: utf-8

import sys
from socket import *
import struct
import move_pb2
import cast_skill_pb2
import move_direct_pb2
import raid_pb2
import team_pb2
import datetime
import get_one_msg

WATCH_PLAYER = {8589935316}

HOST='127.0.0.1'
PORT=13697
PORT=get_one_msg.get_dumpsrv_port()
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
#    print "msgid[%d] player[%lu]" % (msg_id, player_id)
#    data_len = data_len - 8 - 16
#    msg_format = "=IHH" + str(data_len) + 'sQIHH' 
#    msg_len, msg_id, seq, pb_data, player_id, t1, t1, t1 = struct.unpack(msg_format, data)

#    print "read msg:", msg_id	

#    if not player_id in WATCH_PLAYER:
#        continue;

#副本结算
    if msg_id == 10812:
        req = raid_pb2.raid_finish_notify()
        req.ParseFromString(pb_data)
        oldtime=datetime.datetime.now()        
        print oldtime.time(), "gold[%d] exp[%d]" % (req.gold, req.exp)

#星级变化
    if msg_id == 10825:
        req = raid_pb2.raid_star_changed_notify()
        req.ParseFromString(pb_data)
        oldtime=datetime.datetime.now()        
        print oldtime.time(), "[%s] [%s]" % (req.star, req.param)
        
