#!/usr/bin/python
# coding: UTF-8
import os
import sys
import urllib2    
import partner_pb2

if len(sys.argv) != 2:
    print "input url"
    sys.exit(0)

url = (sys.argv[1])
all_the_text = urllib2.urlopen(url=url).read()

req = partner_pb2.PartnerData()
req.ParseFromString(all_the_text)
print "attrs"
for ite in req.attrs:
    if ite.id == 89:
        print "体质: %s" % (ite.val)
    if ite.id == 91:
        print "敏捷: %s" % (ite.val)
    
     
