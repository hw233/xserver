#!/usr/bin/env python                                                                                                                                                                                                    
# -*- coding: UTF-8 -*- 

import urllib
import urllib2
import json
import chardet

import comm_message_pb2

def encode(s):
    return ' '.join([bin(ord(c)).replace('0b', '') for c in s])

def decode(s):
    return ''.join([chr(i) for i in [int(b, 2) for b in s.split(' ')]])

attr_stru = comm_message_pb2.AttrData()
attr_stru.id = 1
attr_stru.val = 20.34

test_data = {'open_id':'163'}
test_data['data'] = attr_stru.SerializeToString()
print encode(test_data['data'])
print len(test_data['data'])
print type(test_data['data'])
print (chardet.detect(test_data['data'])['encoding'])

#attr_stru2 = comm_message_pb2.AttrData()
#attr_stru2.ParseFromString(test_data['data'])
#print "stru2 : %s" % (attr_stru2)
#'''
test_data_urlencode = urllib.urlencode(test_data)

requrl = "http://192.168.2.114/cgi-bin/cgi_save_setting.py"

print test_data

req = urllib2.Request(url = requrl,data =test_data_urlencode)
print "req:%s" % (req)

res_data = urllib2.urlopen(req)
res = res_data.read()
print "res:%s" % (res)
#'''

