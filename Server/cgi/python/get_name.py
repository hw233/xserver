#!/usr/bin/env python
# -*- coding: UTF-8 -*- 

import MySQLdb
import cgi

print "Content-type:text/html\n"
#print "<html>"
#print "<head>"
#print "<meta charset=utf-8>"
#print "</head>"

form = cgi.FieldStorage()

server_id = form.getvalue('server_id')
if not server_id:
    print "{\"code\": 1, \"reason\": \"缺少server_id参数\"}"
    exit()

sex = form.getvalue('sex')
if not sex:
    print "{\"code\": 1, \"reason\": \"缺少sex参数\"}"
    exit()

'''
server_id = 3
sex = 1
'''

try:
    conn = MySQLdb.connect(host='127.0.0.1',user='root',passwd='123456', db='xgame_name',port=3306,charset='utf8')
    if not conn:
        print "{\"code\": 2, \"reason\": \"连接数据库失败\"}"
        exit()

    cursor = conn.cursor()
    if not cursor:
        print "{\"code\": 2, \"reason\": \"连接数据库失败\"}"
        exit()
        
    sql = "select id,name from name%s where sex=%s limit 1;" % (server_id, sex)
    cursor.execute(sql)
    data = cursor.fetchone()
    if not data:
        print "{\"code\": 2, \"reason\": \"名字库已空\"}"
        exit()
    print "{\"code\": 0, \"reason\": \"OK\", \"id\":\"%ld\", \"name\":\"%s\"}" % (data[0], data[1].encode('utf-8'))
    sql = "delete from name%s where id=%ld;" % (server_id, data[0])
    cursor.execute(sql)
    conn.commit()

    cursor.close()
    conn.close()

except MySQLdb.Error,e:
    print "\"code\": 10, \"reason\": \"Mysql Error %d: %s\"" % (e.args[0], e.args[1])
except SyntaxError, e:
    print "\"code\": 20, \"reason\":\"Syntax Error %d: %s\"" % (e.args[0], e.args[1])


#print "</html>"

