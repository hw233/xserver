#!/usr/bin/python
# coding: UTF-8
import redis
import sys
import wanyaogu_pb2

key = "wyk_" + sys.argv[1]
#print "key = ", key

r = redis.Redis()

a = r.hget('server_2', key)
if a:
    pb = wanyaogu_pb2.list_wanyaoka_answer()
    pb.ParseFromString(a)
    print pb.wanyaoka_id
else:
    print 'None'
