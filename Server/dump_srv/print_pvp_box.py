#!/usr/bin/python
# coding: UTF-8

import sys
from socket import *
import struct
import raid_pb2
import pvp_raid_pb2
import role_pb2
import login_pb2
import cast_skill_pb2
import move_direct_pb2
import move_pb2
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

#11705  //段位积分变化通知 pvp_score_changed_notify
    if msg_id == 11705:  
        req = pvp_raid_pb2.pvp_score_changed_notify()
        req.ParseFromString(pb_data)
        box_buf = ""
        for ite in req.avaliable_box_id:
            box_buf = box_buf + str(ite) + " "
        
        oldtime=datetime.datetime.now()
        print  oldtime.time(), ": type[%u] max_lv[%u] today_win[%u] reward_lv[%u] [%s]" % (req.type, req.max_level, req.today_win_num, req.avaliable_reward_level, box_buf)

#11718  //结算
    if msg_id == 11718:  
        req = pvp_raid_pb2.pvp_raid_finished_notify()
        req.ParseFromString(pb_data)
        
        oldtime=datetime.datetime.now()
        print  oldtime.time(), ": win_kill[%u] lose_kill[%u] score_changed[%d] gold_changed[%d]" % (req.win_kill, req.lose_kill, req.score_changed, req.courage_gold_changed)

