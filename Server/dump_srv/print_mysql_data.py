#!/usr/bin/python
# coding: UTF-8

import sys
import MySQLdb
import player_db_pb2


HOST='127.0.0.1'
PORT=3306
USER='root'
PASSWD='123456'
SERVERID=2
PLAYERID=8589935260


server_key = 'xgame%d' % SERVERID
r = MySQLdb.Connect(host=HOST, port = PORT, user=USER, passwd = PASSWD, db=server_key)
cur = r.cursor()
sql = 'select comm_data from player where player_id = %lu' % PLAYERID
cur.execute(sql)
data = cur.fetchone()
pb_data = data[0]

def print_attr(title, t_id, id, value):
    attr_len = len(id)
    print 'attrlen = %d' % attr_len
    for i in range(attr_len):
        print 'id = %d value = %d' % (id[i], value[i])
        if id[i] != t_id:
            continue
        print '%s%d' % (title, value[i])
        break

req = player_db_pb2.PlayerDBInfo()
req.ParseFromString(pb_data)
print 'playerid : %lu' % PLAYERID
print 'coin: %d' % req.coin
print 'exp : %lu' % req.exp
print 'scene : %lu' % req.scene_id
print 'x : %lu' % req.pos_x
print 'z : %lu' % req.pos_z

