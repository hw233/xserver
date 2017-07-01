#!/usr/bin/env python
# -*- coding: UTF-8 -*- 

import MySQLdb
import cgi
import urllib
import chardet
import json
import base64

print "Content-type:text/html\n"

form = cgi.FieldStorage()

open_id = form.getvalue('open_id')
if not open_id:
    print "{\"code\": 1, \"reason\": \"缺少open_id参数\"}"
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
        
    sql = "select data from setting where open_id = %s;" % (open_id)
    n = cursor.execute(sql)
    row = cursor.fetchone()
    if not row:
        print "{\"code\": 2, \"reason\": \"执行SQL命令出错\"}"
        exit()
#    print "{\"code\": 0, \"reason\": \"OK\", \"open_id\":\"%s\", \"data\":\"%s\"}" % (open_id, row[0])
#    '''
    resp = {'code':0, 'reason':'OK'}
    resp['open_id'] = open_id
    resp['data'] = row[0] #.decode('windows-1252').encode('utf-8')
    print json.dumps(resp)
#    '''
    conn.commit()


    cursor.close()
    conn.close()

except MySQLdb.Error,e:
    print "{\"code\": 10, \"reason\": \"Mysql Error %d: %s\"}" % (e.args[0], e.args[1])
except SyntaxError, e:
    print "{\"code\": 20, \"reason\":\"Syntax Error %d: %s\"}" % (e.args[0], e.args[1])


