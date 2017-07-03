#!/usr/bin/python
# coding: UTF-8

import sys
from socket import *
import struct
import move_pb2
import role_pb2
import login_pb2
import cast_skill_pb2
import move_direct_pb2
import team_pb2
import datetime
import get_one_msg
import scene_transfer_pb2
import horse_pb2

WATCH_PLAYER = {8589935372}
WATCH_ATTR_ID = {67, 68}

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

#登陆
    if msg_id == 10007:
        req = login_pb2.EnterGameAnswer()
        req.ParseFromString(pb_data)
        oldtime=datetime.datetime.now()
        print  oldtime.time(), ": %lu 进入游戏 y[%s %.1f %.1f] [%s]" % (player_id, req.posY, req.posX, req.posZ, req.direct)
        
#场景切换
    if msg_id == 10112:        
        req = scene_transfer_pb2.scene_transfer_answer()
        req.ParseFromString(pb_data)
        oldtime=datetime.datetime.now()
        print  oldtime.time(), ": %lu 场景切换[%s] y[%s]" % (player_id, req.new_scene_id , req.pos_y)

#上坐骑
    if msg_id == 11207:        
        req = horse_pb2.OnHorseRequest()
        req.ParseFromString(pb_data)
        oldtime=datetime.datetime.now()
        print  oldtime.time(), ": %lu 上坐骑 y[%s]" % (player_id, req.pos_y)

#下坐骑
    if msg_id == 11209:        
        oldtime=datetime.datetime.now()
        print  oldtime.time(), ": %lu 下坐骑" % (player_id)

        
#y移动开始
    if msg_id == 10114:        
        req = move_direct_pb2.move_y_start_request()
        req.ParseFromString(pb_data)
        oldtime=datetime.datetime.now()
        print  oldtime.time(), ": %lu y移动开始 y[%s]" % (player_id, req.cur_pos_y)

#y移动停止
    if msg_id == 10116:        
        req = move_direct_pb2.move_y_stop_request()
        req.ParseFromString(pb_data)
        oldtime=datetime.datetime.now()
        print  oldtime.time(), ": %lu y移动结束 y[%s]" % (player_id, req.cur_pos_y)

#坐骑属性变更
    if msg_id == 10400:        
        req = role_pb2.PlayerAttrNotify()
        req.ParseFromString(pb_data)
        oldtime=datetime.datetime.now()
        for t1 in req.attrs:
            if not t1.id in WATCH_ATTR_ID:
                continue;
            print oldtime.time(), ": 坐骑属性变更 %lu attr[%s] to [%s]" % (req.player_id, t1.id, t1.val)
