#!/usr/bin/python
# coding: UTF-8

import sys
from socket import *
import raid_pb2
import move_pb2
import cast_skill_pb2
import move_direct_pb2
import role_pb2
import partner_pb2
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

#MSG_ID_PARTNER_RECRUIT_REQUEST 13117        //伙伴招募请求 PartnerRecruitRequest
    if msg_id == 10124:
	req = partner_pb2.PartnerRecruitRequest()
	req.ParseFromString(pb_data)
	oldtime=datetime.datetime.now()
	print oldtime.time(), "%lu: type[%lu]" % (player_id, req.type)

#MSG_ID_PARTNER_RECRUIT_ANSWER 13118         //伙伴招募应答 PartnerRecruitAnswer        
