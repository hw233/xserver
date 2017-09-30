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

while True:
    ret, last_data, player_id, msg_id, pb_data = get_one_msg.get_one_msg(client, last_data)
    if ret == -1:
        break
    if ret == 0:
        continue

#视野变化    
    if msg_id == 10103:
#        print "get 10103"
        req = move_pb2.sight_changed_notify()
        req.ParseFromString(pb_data)
        oldtime = datetime.datetime.now()        
        for t1 in req.add_partner:
            print oldtime.time(),
            print ": 添加伙伴",
            print "%lu owner[%lu] pos[%.1f %.1f]" % (t1.uuid, t1.owner, t1.data[0].pos_x, t1.data[0].pos_z)            

        for t1 in req.delete_partner:
            print oldtime.time(),
            print ": 删除伙伴",            
            print ": %lu" % (t1)
        

