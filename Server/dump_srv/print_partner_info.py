#!/usr/bin/python
# coding: UTF-8

import sys
from socket import *
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

def get_target_data(t1):
    retdata = ""
    for target in t1.target_playerid:
        tmp = "(%lu) " % target
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

    if len(WATCH_PLAYER) != 0 and not player_id in WATCH_PLAYER:
        continue
#    if not player_id in WATCH_PLAYER:
#        continue

#MSG_ID_PARTNER_INFO_ANSWER 13102            //伙伴信息应答 PartnerInfoAnswer
    if msg_id == 13102:
	req = partner_pb2.PartnerInfoAnswer()
	req.ParseFromString(pb_data)
	oldtime=datetime.datetime.now()

        for partner in req.partners:
            print "attrs [%u][%lu]" % (partner.partnerId, partner.uuid)
            for ite in partner.attrs:
                if ite.id == 89:
                    print "体质: %s" % (ite.val)
                if ite.id == 91:
                    print "敏捷: %s" % (ite.val)
            


