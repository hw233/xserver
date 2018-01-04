#!/usr/bin/python
# coding: UTF-8

import sys
from socket import *
import raid_pb2
import move_pb2
import cast_skill_pb2
import move_direct_pb2
import role_pb2
import datetime
import get_one_msg
import get_uuid_type

WATCH_PLAYER = {}
WATCH_UUID = {1125901403839415}

HOST='127.0.0.1'
PORT=13697
PORT=get_one_msg.get_dumpsrv_port()
ADDR=(HOST, PORT)
client=socket(AF_INET, SOCK_STREAM)
client.connect(ADDR)

last_data = ""
player_list = {}

def get_target_data(t1):
    retdata = ""
    for target in t1.target_playerid:
        tmp = "(%lu) " % target
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

    if len(WATCH_PLAYER) != 0 and not player_id in WATCH_PLAYER:
        continue
#    if not player_id in WATCH_PLAYER:
#        continue

#MSG_ID_MOVE_NOTIFY  			10102 //移动通知
    if msg_id == 10102:
	req = move_pb2.move_notify()
	req.ParseFromString(pb_data)
	oldtime=datetime.datetime.now()

        if get_uuid_type.get_uuid_type(req.playerid) == get_uuid_type.ENTITY_TYPE_PARTNER:
	    print oldtime.time(), "%lu: partner move" % (req.playerid)

        
#MSG_ID_SKILL_CAST_NOTIFY
    if msg_id == 10202:
	req = cast_skill_pb2.skill_cast_notify()
	req.ParseFromString(pb_data)
	oldtime=datetime.datetime.now()
 
	if get_uuid_type.get_uuid_type(req.playerid) != get_uuid_type.ENTITY_TYPE_PARTNER:
	    continue
	print oldtime.time(), "[%lu] attack [%u] pos[%.1f][%.1f] target_pos[%.1f %.1f]" % (req.playerid, req.skillid, req.cur_pos.pos_x, req.cur_pos.pos_z, req.target_pos.pos_x, req.target_pos.pos_z)

#MSG_ID_SKILL_HIT_NOTIFY
    if msg_id == 10205:
	req = cast_skill_pb2.skill_hit_notify()
	req.ParseFromString(pb_data)
	oldtime=datetime.datetime.now()
 
	if len(req.target_player) == 0:
	    continue;
	if get_uuid_type.get_uuid_type(req.playerid) == get_uuid_type.ENTITY_TYPE_PARTNER:
	    print oldtime.time(), "[%lu] hit skill[%u] pos[%.1f][%.1f] [%s %s]" % (req.playerid, req.skillid, req.attack_pos.pos_x, req.attack_pos.pos_z, req.target_player[0].hp_delta, req.target_player[0].cur_hp)

#MSG_ID_SKILL_HIT_IMMEDIATE_NOTIFY
    if msg_id == 10206:
	req = cast_skill_pb2.skill_hit_immediate_notify()
	req.ParseFromString(pb_data)
	cur_hp = req.target_player.cur_hp	 
	oldtime=datetime.datetime.now()
	if get_uuid_type.get_uuid_type(req.playerid) == get_uuid_type.ENTITY_TYPE_PARTNER:
	    print oldtime.time(), "[%lu] skill[%u] hit immediate pos[%.1f][%.1f]" % (req.playerid, req.skillid, req.attack_pos.pos_x, req.attack_pos.pos_z);

