#!/usr/bin/python
# coding: UTF-8

import sys
from socket import *
import struct
import raid_pb2
import pk_pb2
import role_pb2
import login_pb2
import cast_skill_pb2
import move_direct_pb2
import team_pb2
import datetime
import get_one_msg
import scene_transfer_pb2
import horse_pb2
import comm_message_pb2

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

#设置PK类型 11601
    if msg_id == 11601:
        req = pk_pb2.set_pk_type_request()
        req.ParseFromString(pb_data)
        oldtime=datetime.datetime.now()
        print  oldtime.time(), ": %lu 设置pk类型请求[%s]" % (player_id, req.pk_type)        
    
    if msg_id == 11602:
        req = comm_message_pb2.comm_answer()
        req.ParseFromString(pb_data)
        oldtime=datetime.datetime.now()
        print  oldtime.time(), ": %lu 设置pk类型返回[%s]" % (player_id, req.result)        
    
#发起切磋
    if msg_id == 11603:
        req = pk_pb2.qiecuo_request()
        req.ParseFromString(pb_data)
        oldtime=datetime.datetime.now()
        print  oldtime.time(), ": %lu 请求切磋[%s]" % (player_id, req.player_id)                

#同意切磋 11606
    if msg_id == 11606:
        req = pk_pb2.qiecuo_start_request()
        req.ParseFromString(pb_data)
        oldtime=datetime.datetime.now()
        print  oldtime.time(), ": %lu 同意切磋[%s]" % (player_id, req.player_id)                

#切磋结束  11610
    if msg_id == 11610:
        req = pk_pb2.qiecuo_finish_notify()
        req.ParseFromString(pb_data)
        oldtime=datetime.datetime.now()
        print  oldtime.time(), ": %lu 切磋结束[%lu]" % (player_id, req.result)                

#杀戮值属性变更
    if msg_id == 10400:
        req = role_pb2.PlayerAttrNotify()
        req.ParseFromString(pb_data)
        oldtime=datetime.datetime.now()
        for t1 in req.attrs:
            if t1.id != 70:
                continue;
            print oldtime.time(), ": player %lu %lu attr[%s] to [%s]" % (player_id, req.player_id, t1.id, t1.val)

#命中广播
    if msg_id == 10205:
        req = cast_skill_pb2.skill_hit_notify()
        req.ParseFromString(pb_data)
        owner_name = req.ownername
        for t1 in req.target_player:
            print "10205: [%s][%s][%s] hit notify id[%lu] hpdelta[%d] hp[%d]" % (req.playerid, req.owneriid, owner_name, t1.playerid, t1.hp_delta, t1.cur_hp)
                                                                                            
            
    if msg_id == 10206:
        req = cast_skill_pb2.skill_hit_immediate_notify()
        req.ParseFromString(pb_data)
        owner_name = req.ownername
        t1 = req.target_player
        print "10206: [%s][%s][%s] hit notify id[%lu] hpdelta[%d] hp[%d]" % (req.playerid, req.owneriid, owner_name, t1.playerid, t1.hp_delta, t1.cur_hp)
        
            
