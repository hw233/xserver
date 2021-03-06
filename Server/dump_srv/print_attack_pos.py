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

WATCH_PLAYER = {8589936794}
WATCH_UUID = {1125901403839415}

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

    #心跳
    if msg_id == 10009:
        continue
    
#    if not player_id in WATCH_PLAYER:
#        continue

#    print 'msgid = ', msg_id

#MSG_ID_MOVE_NOTIFY  			10102 //移动通知
#    if msg_id == 10102:
#        req = move_pb2.move_notify()
#        req.ParseFromString(pb_data)
#        oldtime=datetime.datetime.now()
#        if get_uuid_type.get_uuid_type(req.playerid) != get_uuid_type.ENTITY_TYPE_MONSTER:
#            continue
# 
#        if not req.playerid in WATCH_UUID:
#            continue
#        
#        if len(req.data) == 0:
#            continue
#        movedata=""
#        for t1 in req.data:
#            movedata = movedata + " [%.1f %.1f]" % (t1.pos_x, t1.pos_z)
##            b.append((t1.pos_x, t1.pos_z))
#        print oldtime.time(), "%lu: movedata %s" % (req.playerid, movedata)

#MSG_ID_MOVE_START_REQUEST 			10105 //移动开始请求
#    if msg_id == 10105:
#        req = move_direct_pb2.move_start_request()
#        req.ParseFromString(pb_data)
#        oldtime=datetime.datetime.now()
#        print oldtime.time(), "[%lu] move_start_request cur_pos[%.1f][%.1f] direct[%.1f %.1f]" % \
#            (player_id, req.cur_pos.pos_x, req.cur_pos.pos_z, req.direct_x, req.direct_z)

#MSG_ID_MOVE_STOP_REQUEST 			10108 //移动停止请求
#    if msg_id == 10108:
#        req = move_direct_pb2.move_stop_request()
#        req.ParseFromString(pb_data)
#        oldtime=datetime.datetime.now()
#        print oldtime.time(), "[%lu] move_stop_request cur_pos[%.1f][%.1f]" % \
#            (player_id, req.cur_pos.pos_x, req.cur_pos.pos_z)

#MSG_ID_SKILL_CAST_REQUEST 		10200  //施法请求  skill_cast_request
#    if msg_id == 10200:
#        req = cast_skill_pb2.skill_cast_request()
#        req.ParseFromString(pb_data)
#        oldtime=datetime.datetime.now()
#        print oldtime.time(), "[%lu] skill_cast_request [%u] pos[%.1f][%.1f] direct[%.1f %.1f]" % \
#            (player_id, req.skillid, req.cur_pos.pos_x, req.cur_pos.pos_z, req.direct_x, req.direct_z)

#MSG_ID_SKILL_CAST_NOTIFY
    if msg_id == 10202:
	req = cast_skill_pb2.skill_cast_notify()
	req.ParseFromString(pb_data)
	oldtime=datetime.datetime.now()
 
	print oldtime.time(), "[%lu] attack [%u] pos[%.1f][%.1f] direct[%.1f %.1f]" % (req.playerid, req.skillid, req.cur_pos.pos_x, req.cur_pos.pos_z, req.direct_x, req.direct_z)
 
#	if get_uuid_type.get_uuid_type(req.playerid) != get_uuid_type.ENTITY_TYPE_MONSTER:
#	    continue
#	print oldtime.time(), "[%lu] attack [%u] pos[%.1f][%.1f] target_pos[%.1f %.1f]" % (req.playerid, req.skillid, req.cur_pos.pos_x, req.cur_pos.pos_z, req.target_pos.pos_x, req.target_pos.pos_z)
#	print "have target_pos = %s" % (req.HasField('target_pos'))

#MSG_ID_SKILL_HIT_NOTIFY
#    if msg_id == 10205:
#        req = cast_skill_pb2.skill_hit_notify()
#        req.ParseFromString(pb_data)
#        oldtime=datetime.datetime.now()
# 
#        print oldtime.time(), "[%lu] hit skill[%u] pos[%.1f][%.1f]" % (req.playerid, req.skillid, req.attack_pos.pos_x, req.attack_pos.pos_z)
#	if len(req.target_player) == 0:
#	    continue;
#	if get_uuid_type.get_uuid_type(req.playerid) == get_uuid_type.ENTITY_TYPE_MONSTER:
#	    print oldtime.time(), "[%lu] hit skill[%u] pos[%.1f][%.1f] [%s %s]" % (req.playerid, req.skillid, req.attack_pos.pos_x, req.attack_pos.pos_z, req.target_player[0].hp_delta, req.target_player[0].cur_hp)

#MSG_ID_SKILL_HIT_IMMEDIATE_NOTIFY
#    if msg_id == 10206:
#        req = cast_skill_pb2.skill_hit_immediate_notify()
#        req.ParseFromString(pb_data)
#        cur_hp = req.target_player.cur_hp        
#        oldtime=datetime.datetime.now()
#        if get_uuid_type.get_uuid_type(req.playerid) == get_uuid_type.ENTITY_TYPE_MONSTER:
#            print oldtime.time(), "[%lu] hit immediate pos[%.1f][%.1f]" % (req.playerid, req.attack_pos.pos_x, req.attack_pos.pos_z);

