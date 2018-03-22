#!/usr/bin/python
# coding: UTF-8
import os
import sys
import urllib2    
import comm_message_pb2

if len(sys.argv) != 2:
    print "input url"
    sys.exit(0)

url = (sys.argv[1])
all_the_text = urllib2.urlopen(url=url).read()


req = comm_message_pb2.EquipData()
req.ParseFromString(all_the_text)
print "%u %u %u" % (req.type, req.stair, req.starLv)
     
