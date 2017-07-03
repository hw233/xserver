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
import datetime
import get_one_msg
import scene_transfer_pb2
import horse_pb2

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

#场景切换 10112
    if msg_id == 10112:
        req = scene_transfer_pb2.scene_transfer_answer();
        req.ParseFromString(pb_data)
        oldtime=datetime.datetime.now()
        print  oldtime.time(), ": %lu 进入场景[%s]" % (player_id, req.new_scene_id)        
    
    
#副本完成 10812
    if msg_id == 10812:
        req = raid_pb2.raid_finish_notify();
        req.ParseFromString(pb_data)
        oldtime=datetime.datetime.now()
        print  oldtime.time(), ": %lu 副本结算[%s]" % (player_id, req.star)                

#万妖卡列表  11401
    if msg_id == 11401:
        req = wanyaogu_pb2.list_wanyaoka_answer()
        req.ParseFromString(pb_data)
        oldtime=datetime.datetime.now()
        print  oldtime.time(), ": %lu 万妖卡列表[%s]" % (player_id, req.wanyaoka_id)                

#万妖谷关卡开始通知  11402
    if msg_id == 11402:
        req = wanyaogu_pb2.wanyaogu_start_notify()
        req.ParseFromString(pb_data)
        oldtime=datetime.datetime.now()
        print  oldtime.time(), ": %lu 万妖谷开始[%lu]" % (player_id, req.start_time)                

#万妖谷关卡火炉挂机通知 11403
    if msg_id == 11403:
        oldtime=datetime.datetime.now()
        print  oldtime.time(), ": %lu 万妖谷火炉挂机" % (player_id)

#进入游戏对时
    if msg_id == 10007:
        req = login_pb2.EnterGameAnswer()
        req.ParseFromString(pb_data)        
        oldtime=datetime.datetime.now()
        print  oldtime.time(), ": %lu 进入游戏 %u %d [%u %s %s]" % (player_id, req.curTime, req.direct, req.sceneId, req.posX, req.posZ)        

#11404 //获得万妖卡通知 wanyaoka_get_notify
    if msg_id == 11404:
        req = wanyaogu_pb2.wanyaoka_get_notify()
        req.ParseFromString(pb_data)        
        oldtime=datetime.datetime.now()
        print  oldtime.time(), ": %lu 获得万妖卡 %s" % (player_id, req.wanyaoka_id)
