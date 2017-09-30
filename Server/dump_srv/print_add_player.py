#!/usr/bin/python
# coding: UTF-8

import sys
from socket import *
import struct
import move_pb2
import cast_skill_pb2
import move_direct_pb2
import team_pb2
import horse_pb2
import datetime
import get_one_msg

WATCH_PLAYER = {4294968631, 8589938012, 8589938011}

sight_player = {}


HOST='127.0.0.1'
PORT=13697
PORT=get_one_msg.get_dumpsrv_port()
ADDR=(HOST, PORT)
client=socket(AF_INET, SOCK_STREAM)
client.connect(ADDR)

last_data = ""
player_list = {}

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
#    data_len = data_len - 8 - 16
#    msg_format = "=IHH" + str(data_len) + 'sQIHH' 
#    msg_len, msg_id, seq, pb_data, player_id, t1, t1, t1 = struct.unpack(msg_format, data)

#    print "read msg:", msg_id	

    if player_id not in WATCH_PLAYER:
        continue

#视野变化    
    if msg_id == 10103:
#        print "get 10103 msg playerid[%lu]" % (player_id)
        req = move_pb2.sight_changed_notify()
        req.ParseFromString(pb_data)
        oldtime = datetime.datetime.now()        
        for t1 in req.add_player:
            if sight_player.has_key(t1.playerid):
                print oldtime.time(), "err, add dup player %s [%lu]" % (t1.name, t1.playerid)
                continue
                
            sight_player[t1.playerid] = t1.name
            print oldtime.time(), ": add player[%s] %lu horse[%u] y[%.2f]" % (t1.name, t1.playerid, t1.horse, t1.pos_y)

        for t1 in req.delete_player:
            if sight_player.has_key(t1):
                print oldtime.time(), ": del player %lu[%s]" % (t1, sight_player[t1])
                del sight_player[t1]
            else:
                print oldtime.time(), "err: 删除不存在的玩家 del player %lu" % (t1)

    if msg_id == 11211:
        req = horse_pb2.OnHorseRequest()
        req.ParseFromString(pb_data)
        oldtime = datetime.datetime.now()
        print oldtime.time(), ": %lu onhorse [%s]" % (player_id, req.pos_y)
        
