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
import get_one_msg

WATCH_PLAYER = {12884902964}

HOST='127.0.0.1'
PORT=10697
BUFSIZ=1024
PORT=get_one_msg.get_dumpsrv_port()
ADDR=(HOST, PORT)
client=socket(AF_INET, SOCK_STREAM)
client.connect(ADDR)

last_data = ""
player_list = {}
saved_uuid = 0

def get_buff_data(t1):
    retdata = ""
    for buffinfo in t1.buff_info:
        tmp = "(%d) " % buffinfo.id
        retdata = retdata + tmp
    return retdata

while True:
    ret, last_data, player_id, msg_id, pb_data = get_one_msg.get_one_msg(client, last_data)
    if ret == -1:
        break
    if ret == 0:
        continue

    #心跳
    if msg_id == 10009:
        continue
    
#    if not player_id in WATCH_PLAYER:
#        continue;

#skill_cast_request
    if msg_id == 10200:
        req = cast_skill_pb2.skill_cast_request()
        req.ParseFromString(pb_data)
        oldtime=datetime.datetime.now()        
        print oldtime.time() , "%.1f %.1f, direct %.1f %.1f" % (req.cur_pos.pos_x, req.cur_pos.pos_z, req.direct_x, req.direct_z)

#skill_hit_notify
    if msg_id == 10205:
        req = cast_skill_pb2.skill_hit_notify()
        req.ParseFromString(pb_data)
        if len(req.target_player) == 0:
            continue
        
        oldtime=datetime.datetime.now()
        for target in req.target_player:
            print oldtime.time() , "hit %.1f %.1f add buff[%s]" % (target.target_pos.pos_x, target.target_pos.pos_z, target.add_buff)

#skill_hit_immediate_notify        
#    if msg_id == 10206:
#        req = cast_skill_pb2.skill_hit_immediate_notify()
#        req.ParseFromString(pb_data)
#        logdata = ""
#        t1 = req.target_player
#        for t2 in t1.add_buff:
#            logdata = logdata + " buff[%u]" % t2
#        logdata = logdata + " move to [%.1f][%.1f]" % (t1.target_pos.pos_x, t1.target_pos.pos_z)
#        oldtime=datetime.datetime.now()
#        print oldtime.time() , "怪打人", logdata
# 
##
#    if msg_id == 10108:
#        req = move_direct_pb2.move_stop_request()
#        req.ParseFromString(pb_data)
#        oldtime=datetime.datetime.now()
#        print oldtime.time() , "stopmove %.1f %.1f" % (req.cur_pos.pos_x, req.cur_pos.pos_z)
# 
# 
##move_notify        
#    if msg_id == 10102:
#        req = move_pb2.move_notify()
#        req.ParseFromString(pb_data)
#        uuid = req.playerid
#        if len(req.data) == 0:
#            continue
#        movedata=""
#        for t1 in req.data:
#            movedata = movedata + " [%.1f %.1f]" % (t1.pos_x, t1.pos_z)
# 
#        oldtime=datetime.datetime.now()
##        print oldtime.time() , "[%lu]: movedata %s" % (uuid, movedata)
