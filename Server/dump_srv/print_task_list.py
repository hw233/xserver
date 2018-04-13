#!/usr/bin/python
# coding: UTF-8

import sys
from socket import *
import raid_pb2
import task_pb2
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

def get_task_data(t1):
    retdata = ""
    for taskdata in t1.ongoing_list:
        tmp = "(%d) " % taskdata.id
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

#MSG_ID_TASK_LIST_REQUEST 10601                  //任务列表请求 NULL
    if msg_id == 10601:
	oldtime=datetime.datetime.now()        
	print oldtime.time(), "%lu: request task list" % (player_id)

        
#MSG_ID_TASK_LIST_ANSWER 10602                   //任务列表应答 TaskListAnswer
    if msg_id == 10602:
	req = task_pb2.TaskListAnswer()
	req.ParseFromString(pb_data)
	oldtime=datetime.datetime.now()
        ongotask = get_task_data(req)
	print oldtime.time(), "%lu: task list ret[%d] ongoing[%s] finish_list[%s] chapterid[%u] chapterstate[%u]" % (player_id, req.result, ongotask, req.finish_list, req.chapterId, req.chapterState)

#MSG_ID_TASK_ACCEPT_REQUEST 10603                //接任务请求 TaskCommRequest
    if msg_id == 10603:
	req = task_pb2.TaskCommRequest()
	req.ParseFromString(pb_data)
	oldtime=datetime.datetime.now()
	print oldtime.time(), "%lu: task accept id[%u]" % (player_id, req.task_id)
        
#MSG_ID_TASK_UPDATE_NOTIFY 10600                 //任务更新通知 TaskUpdateNotify
    if msg_id == 10600:
	req = task_pb2.TaskUpdateNotify()
	req.ParseFromString(pb_data)
	oldtime=datetime.datetime.now()
        taskdata = req.data
	print oldtime.time(), "%lu: task update id[%u] status[%u] expire[%u]" % (player_id, taskdata.id, taskdata.status, taskdata.expireTime)

