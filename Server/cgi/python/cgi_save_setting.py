#!/usr/bin/env python
# -*- coding: UTF-8 -*- 

import MySQLdb
import cgi

print "Content-type:text/html\n"

form = cgi.FieldStorage()

open_id = form.getvalue('open_id')
if not open_id:
    print "{\"code\": 1, \"reason\": \"缺少open_id参数\"}"
    exit()
bin_data = form.getvalue('data')
if not bin_data:
    print "{\"code\": 1, \"reason\": \"缺少data参数\"}"
    exit()


try:
    conn = MySQLdb.connect(host='127.0.0.1',user='root',passwd='123456', db='xgame_global',port=3306,charset='utf8')
    if not conn:
        print "{\"code\": 2, \"reason\": \"连接数据库失败\"}"
        exit()

    cursor = conn.cursor()
    if not cursor:
        print "{\"code\": 2, \"reason\": \"连接数据库失败\"}"
        exit()
        
    sql = "replace setting set open_id = %s, data = \'%s\';" % (open_id, MySQLdb.escape_string(bin_data))
#    print sql
    try:
        cursor.execute(sql)
    except MySQLdb.Error, e:
        print "{\"code\": 2, \"reason\": \"执行SQL命令出错(%s)\"}" % (e)
        exit()
    print "{\"code\": 0, \"reason\": \"OK\", \"open_id\":\"%s\"}" % (open_id)
    conn.commit()


    cursor.close()
    conn.close()

except MySQLdb.Error,e:
    print "{\"code\": 10, \"reason\": \"Mysql Error %d: %s\"}" % (e.args[0], e.args[1])
except SyntaxError, e:
    print "{\"code\": 20, \"reason\":\"Syntax Error %d: %s\"}" % (e.args[0], e.args[1])



