#!/usr/bin/python
# coding: UTF-8
import os
import sys
import comm_message_pb2

if len(sys.argv) != 2:
    print "input file name"
    sys.exit(0)

filename = (sys.argv[1])
binfile=open(filename,'rb')

try:
     all_the_text = binfile.read( )

finally:
     binfile.close()

req = comm_message_pb2.EquipData()
req.ParseFromString(all_the_text)
print "%u %u %u" % (req.type, req.stair, req.starLv)
     
