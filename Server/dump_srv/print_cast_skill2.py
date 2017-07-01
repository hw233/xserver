#!/usr/bin/python
# coding: UTF-8

import sys
from socket import *
import struct
import move_pb2
import cast_skill_pb2
import move_direct_pb2
import team_pb2
import datetime
import get_one_msg

#WATCH_PLAYER = {8589935316}
#WATCH_PLAYER = {0x2000002e0}
WATCH_PLAYER = {8589935329}

HOST='127.0.0.1'
PORT=11697
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

#    if not player_id in WATCH_PLAYER:
#        continue;

#MSG_ID_SKILL_HIT_NOTIFY
    if msg_id == 10205:
        req = cast_skill_pb2.skill_hit_notify()
        req.ParseFromString(pb_data)
        if len(req.target_player) == 0:
            continue
        
        damage = req.target_player[0].hp_delta;
        cur_hp = req.target_player[0].cur_hp
        oldtime=datetime.datetime.now()        
        print oldtime.time(), "attack life_steal[%u] damage_return[%u] cur_hp[%d] damage[%u] target_cur_hp[%d]" % (req.life_steal, req.damage_return, req.attack_cur_hp, damage, cur_hp);
        
#MSG_ID_SKILL_HIT_IMMEDIATE_NOTIFY
    if msg_id == 10206:
        req = cast_skill_pb2.skill_hit_immediate_notify()
        req.ParseFromString(pb_data)
        damage = req.target_player.hp_delta;
        cur_hp = req.target_player.cur_hp        
        oldtime=datetime.datetime.now()        
        print oldtime.time(), "monster attack life_steal[%u] damage_return[%u] cur_hp[%d] damage[%u] target_cur_hp[%d]" % (req.life_steal, req.damage_return, req.attack_cur_hp, damage, cur_hp);

    
    if msg_id == 10809:
        oldtime=datetime.datetime.now()                
        print oldtime.time(), "离开位面"
