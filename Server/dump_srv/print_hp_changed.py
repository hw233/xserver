#!/usr/bin/python
# coding: UTF-8

import sys
from socket import *
import struct
import raid_pb2
import wanyaogu_pb2
import login_pb2
import cast_skill_pb2
import move_direct_pb2
import team_pb2
import role_pb2
import datetime
import get_one_msg
import scene_transfer_pb2
import horse_pb2
import cast_skill_pb2
import get_uuid_type

WATCH_PLAYER = {8589935415}

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

#    if not player_id in WATCH_PLAYER:
#        continue;

#MSG_ID_SKILL_HIT_NOTIFY
    if msg_id == 10205:
        req = cast_skill_pb2.skill_hit_notify()
        req.ParseFromString(pb_data)
        oldtime=datetime.datetime.now()        

        for ite in req.target_player:
            uuid = ite.playerid
            if get_uuid_type.get_uuid_type(uuid) != get_uuid_type.ENTITY_TYPE_MONSTER:
                continue
            cur_hp = ite.cur_hp
            damage = ite.hp_delta
            print oldtime.time(), "[%lu] attack [%lu] skill[%s] damage[%d] cur_hp[%d] pos[%.1f][%.1f]" % (req.playerid,
                uuid, req.skillid, damage, cur_hp, ite.target_pos.pos_x, ite.target_pos.pos_z);
        
#        if len(req.target_player) == 0:
#            continue
#        
#        cur_hp = req.target_player[0].cur_hp
#        damage = req.target_player[0].hp_delta
#        
#
#        print oldtime.time(), "[%lu] attack [%lu] skill[%s] damage[%d] cur_hp[%d]" % (req.playerid, req.target_player[0].playerid, req.skillid, damage, cur_hp);
#        
#MSG_ID_SKILL_HIT_IMMEDIATE_NOTIFY
    if msg_id == 10206:
        req = cast_skill_pb2.skill_hit_immediate_notify()
        req.ParseFromString(pb_data)
        cur_hp = req.target_player.cur_hp        
        oldtime=datetime.datetime.now()
        if get_uuid_type.get_uuid_type(req.playerid) == get_uuid_type.ENTITY_TYPE_MONSTER:
            print oldtime.time(), "[%lu] attack [%lu] skill[%u] cur_hp[%d] attack_pos[%.1f][%.1f]" % (req.playerid,
                req.target_player.playerid, req.skillid, cur_hp, req.attack_pos.pos_x, req.attack_pos.pos_z);

#属性变更
#    if msg_id == 10400:
#        req = role_pb2.PlayerAttrNotify()
#        req.ParseFromString(pb_data)
#        oldtime=datetime.datetime.now()
#        for t1 in req.attrs:
#            if t1.id != 2:
#                continue;
#            print oldtime.time(), ": player %lu cur_hp [%s]" % (req.player_id, t1.val)

