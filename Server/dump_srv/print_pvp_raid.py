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

WATCH_PLAYER = {8589935561, 8589935575, 8589935577, 8589935596}

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

# 11721  //PVP副本玩家掉落悬崖通知 pvp_raid_player_fall_notify
    if msg_id == 11721:
        req = pvp_raid_pb2.pvp_raid_player_fall_notify()
        req.ParseFromString(pb_data)
        oldtime=datetime.datetime.now()
        print  oldtime.time(), ": player[%u] 掉落悬崖" % (req.player_id)

# 11717   //PVP副本击杀通知 pvp_kill_notify
    if msg_id == 11717:
        req = pvp_raid_pb2.pvp_kill_notify()
        req.ParseFromString(pb_data)

        assist_buf = ""
        for ite in req.assist_player_id:
            assist_buf = assist_buf + str(ite) + " "
        
        oldtime=datetime.datetime.now()
        print oldtime.time(), ": player[%lu] 杀死 [%lu] 助攻[%s]" % (req.kill_player_id, req.dead_player_id, assist_buf)

# 11725 //PVP副本开始通知 pvp_raid_start_notify
    if msg_id == 11725:  
        req = pvp_raid_pb2.pvp_raid_start_notify()
        req.ParseFromString(pb_data)
        oldtime=datetime.datetime.now()
        print  oldtime.time(), ": 副本开始[%u]" % (req.start_time)
        
#PVP副本结算通知 pvp_raid_finish_notify
    if msg_id == 11718:  
        req = pvp_raid_pb2.pvp_raid_finished_notify()
        req.ParseFromString(pb_data)
        oldtime=datetime.datetime.now()
        print  oldtime.time(), ": win_kill[%u] lose_kill[%u] len_win[%d] len_lose[%d]" % (req.win_kill, req.lose_kill, len(req.win_player), len(req.lose_player))
        print "=====win===="
        for ite in req.win_player:
            print "%lu, %s, %u %u %u %u" % (ite.player_id, ite.name, ite.head_icon, ite.kill, ite.dead, ite.assist)
        print "====lose===="            
        for ite in req.lose_player:
            print "%lu, %s, %u %u %u %u" % (ite.player_id, ite.name, ite.head_icon, ite.kill, ite.dead, ite.assist)
        print "=====end===="
    

#视野变化    
    if msg_id == 10103:
        req = move_pb2.sight_changed_notify()
        req.ParseFromString(pb_data)
        oldtime=datetime.datetime.now()        
        for t1 in req.add_player:
            buffdata = get_buff_data(t1)
            print oldtime.time(),
            print ": 添加玩家",
            print "%lu %s buf[%s]" % (t1.playerid, t1.name,  buffdata)            

#        for t1 in req.delete_player:
#            print oldtime.time(), ": %lu del player %lx" % (player_id, t1)
        

# 10211   //添加buff通知，主要是道具buff使用 add_buff_notify
    if msg_id == 10211:
        req = cast_skill_pb2.add_buff_notify()
        req.ParseFromString(pb_data)
        oldtime=datetime.datetime.now()
        print oldtime.time(), "%lu add buff %u %u %u" % (req.playerid, req.buff_id, req.start_time, req.lv)

# 10400 //属性变更通知 playerattrnotify        
    if msg_id == 10400:
        req = role_pb2.PlayerAttrNotify()
        req.ParseFromString(pb_data)
        oldtime=datetime.datetime.now()
        for t1 in req.attrs:
            if t1.id == 2:
                print oldtime.time(), ": player %lu %lu attr[%s] to [%s]" % (player_id, req.player_id, t1.id, t1.val)
            if t1.id == 21:
                print oldtime.time(), ": player %lu %lu attr[%s] to [%s]" % (player_id, req.player_id, t1.id, t1.val)
                
# 10221   // buff状态变化通知, 比如眩晕 buff_state_changed_notify
    if msg_id == 10221:
        req = cast_skill_pb2.buff_state_changed_notify()
        req.ParseFromString(pb_data)
        oldtime=datetime.datetime.now()
        print oldtime.time(), ": player %lu buffstate[%s]" % (req.player_id, req.buff_state)
