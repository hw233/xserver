#!/usr/bin/python
# coding: UTF-8

import sys
import redis
import player_redis_info_pb2


HOST='127.0.0.1'
PORT=6379
SERVERID=2
PLAYERID=8589935260

r = redis.Redis(HOST, PORT)
server_key = 'server_%d' % SERVERID
pb_data = r.hget(server_key, PLAYERID)
req = player_redis_info_pb2.player_redis_info()
req.ParseFromString(pb_data)
print 'playerid : %lu' % PLAYERID
print 'name: %s' % req.name
print 'lv: %d' % req.lv
