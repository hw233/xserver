#!/usr/bin/python
# coding: UTF-8

import sys
from socket import *
import struct
import move_pb2
import cast_skill_pb2
import move_direct_pb2
import role_pb2
import team_pb2
import datetime
import get_one_msg

WATCH_PLAYER = {4294968631}
WATCH_ATTR_ID = {1, 2}


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

#属性变更
    if msg_id == 10400:
        req = role_pb2.PlayerAttrNotify()
        req.ParseFromString(pb_data)
        oldtime=datetime.datetime.now()
        attrid = ''
#        for t1 in req.attrs:
#            attrid = attrid + ' {' + str(t1.id) + ':' + str(t1.val) + '}'
        
#       print oldtime.time(), ": player %lu attr[%d][%s] changed" % (player_id, len(req.attrs), attrid)       
        for t1 in req.attrs:
            if not t1.id in WATCH_ATTR_ID:
                continue;
            print oldtime.time(), ": player %lu %lu attr[%s] to [%s]" % (player_id, req.player_id, t1.id, t1.val)
        
