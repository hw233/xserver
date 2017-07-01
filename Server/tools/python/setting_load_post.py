#!/usr/bin/env python                                                                                                                                                                                                    
# -*- coding: UTF-8 -*- 

import urllib
import urllib2
import json
import chardet
import base64

import comm_message_pb2

def encode(s):
    return ' '.join([bin(ord(c)).replace('0b', '') for c in s])

def decode(s):
    return ''.join([chr(i) for i in [int(b, 2) for b in s.split(' ')]])

test_data = {'open_id':'163'}
test_data_urlencode = urllib.urlencode(test_data)

requrl = "http://192.168.2.114/cgi-bin/cgi_load_setting.py"

print test_data

req = urllib2.Request(url = requrl,data =test_data_urlencode)
print "req:%s" % (req)

res_data = urllib2.urlopen(req)
res = res_data.read()
print "len(res): %s" % (len(res))
print "type(res): %s" % (type(res))
print "res:%s" % (res)
print "encoding(res):%s" % (chardet.detect(res)['encoding'])

#res2 = res.decode('utf-8')
#print "len(res2): %s" % (len(res2))
#print "type(res2): %s" % (type(res2))
#print "res2:%s" % (res2)
#print "encoding(res2):%s" % (chardet.detect(res2)['encoding'])

json_data = json.loads(res)
print "json_data: %s" % (json_data)
if 'data' in json_data:
#    print "original data:"
#    print (len(json_data['data']))
#    print (type(json_data['data']))
#    print (encode(json_data['data']))
#    print "codec data:"
#    str_decode = json_data['data'].encode('utf-8')
#    print (len(str_decode))
#    print (type(str_decode))
#    print (encode(str_decode))
#    '''
    attr_stru = comm_message_pb2.AttrData()
    attr_stru.ParseFromString(json_data['data'].encode('utf-8'))
    print "id:%s, val:%s" % (attr_stru.id, attr_stru.val)
#    '''


