#!/usr/bin/python
# coding: UTF-8

import sys
from socket import *
import rank_pb2
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

def get_attr(t1, id):
    for attr in t1:
        if attr.id == id:
            return attr.val
    return 0

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

# MSG_ID_RANK_INFO_ANSWER 12802   //排行榜信息应答 RankInfoAnswer
    if msg_id == 12802:
	req = rank_pb2.RankInfoAnswer()
	req.ParseFromString(pb_data)
	oldtime=datetime.datetime.now()

        print "======================"
        print oldtime.time(), "[%lu]: rank type[%u]" % (player_id, req.type)

        for it in req.infos:
            baseinfo = it.baseInfo
            zhenying = get_attr(baseinfo.attrs, 47)
            print "player[%lu] name[%s] zhenying[%u] score[%u]" % (baseinfo.playerId, baseinfo.name, zhenying, it.score)


        
