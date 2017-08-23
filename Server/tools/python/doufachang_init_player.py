#!/usr/bin/python
import redis
import os
import sys

if len(sys.argv) != 2:
    print "input insert num"
    sys.exit(0)

insertnum = int(sys.argv[1])

t = os.popen("cat ../../server_info.ini | grep game_srv_id | awk -F '=' '{print $2}'")
serverid = int(t.readlines()[0])
#print serverid

r = redis.Redis()
key = "doufachang_%d" % (serverid)
r.delete(key)
key = "doufachang_rank_%d" % (serverid)
r.delete(key)
key = "doufachang_rank2_%d" % (serverid)
r.delete(key)
key = "doufachang_lock_%d" % (serverid)
r.delete(key)
key = "doufachang_record_%d" % (serverid)
r.delete(key)

key = "server_%d" % (serverid)

results = r.hkeys(key)
rank = 1
index = 0
while rank <= insertnum and index < len(results):
    if not results[index].isdigit():
        index = index + 1
        continue
    
    key = "doufachang_rank_%d" % (serverid)
    r.hset(key, rank, int(results[index]))
    key = "doufachang_rank2_%d" % (serverid)
    r.hset(key, int(results[index]), rank)
    index = index + 1
    rank = rank + 1
    
