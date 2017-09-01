#!/usr/bin/python
# coding: UTF-8

import sys
import struct
import datetime
import MySQLdb
import player_db_pb2

if len(sys.argv) < 3:
    print "usage: %s server_id player_id" % (sys.argv[0])
    exit()

server_id = int(sys.argv[1])
player_id = int(sys.argv[2])

try:
    conn = MySQLdb.connect(host='127.0.0.1',user='root',passwd='123456', db='xgame%d' % (server_id),port=3306,charset='utf8')
    if not conn:
        print "连接数据库失败"
        exit()

    cursor = conn.cursor()
    if not cursor:
        print "连接数据库失败"
        exit()
        
    sql = "select `open_id`,`player_name`,`lv`,`job`,`logout_time`,`create_time`,`comm_data` from player where `player_id` = %ld;" % (player_id)
    cursor.execute(sql)
    data = cursor.fetchone()
    if not data:
        print "hasn't player %ld" % (player_id)
        exit()
    
    open_id = data[0]
    player_name = data[1]
    lv = data[2]
    job = data[3]
    logout_time = data[4]
    create_time = data[5]
    print "player[%ld], open_id[%s], player_name[%s], lv[%s], job[%s], logout_time[%s], create_time[%s]" % (player_id, open_id, player_name, lv, job, logout_time, create_time)

    db_info = player_db_pb2.PlayerDBInfo()
    db_info.ParseFromString(data[6])
    oldtime=datetime.datetime.now()
    print "task_list: "
    for task in db_info.task_list:
        string = ''
        for count in task.progress:
            string = string + ' %s' % (count.num)
        print '{id:%s, status:%s, accept_ts:%s, accu_time:%s, progress:[%s]}' % (task.id, task.status, task.accept_ts, task.accu_time, string)
    print "task_finish: %s" % (db_info.task_finish)
        
    conn.commit()

    cursor.close()
    conn.close()

except MySQLdb.Error,e:
    print "Mysql Error %d: %s" % (e.args[0], e.args[1])
except SyntaxError, e:
    print "Syntax Error %d: %s" % (e.args[0], e.args[1])

