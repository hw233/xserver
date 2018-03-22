#!/usr/bin/python
# coding: UTF-8

import sys
from socket import *
import struct
import get_one_msg
import mail_db_pb2
'''
import comm_message_pb2
import bag_pb2
'''

#WATCH_PLAYER = {12884902859}

HOST='127.0.0.1'
PORT=get_one_msg.get_dumpsrv_port()
BUFSIZ=1024
ADDR=(HOST, PORT)
client=socket(AF_INET, SOCK_STREAM)
client.connect(ADDR)

#last_data = ""
#player_list = {}

req = mail_db_pb2.MailDBInfo()
req.type = 270300014
req.title = u'测试邮件'
req.senderName = u'系统'
req.content = u'这是一封测试邮件，如果超过32个字，会怎么样呢？\n巴索罗米*熊：要是去旅行的话，你想去哪里？'
'''
req.title = "Message"
req.senderName = "System"
req.content = "This is a test mail."
'''
for i in range(1):
    attach = req.attach.add()
    attach.id = 201020001 + i
    attach.num = 2

req.statisId = 1

seri_str = req.SerializeToString()

msg_len = 8 + 8 + 4 + len(seri_str)
msg_id = 1600
player_id = 42949673559

data = struct.pack('=IHHQI', msg_len, msg_id, 0, player_id, len(seri_str)) + seri_str


send_len = client.send(data)
print "msg_len:%s msg_id:%s player_id:%s len(data):%s" % (msg_len, msg_id, player_id, len(data))
exit(0)

'''
while True:
    data=client.recv(BUFSIZ)
    data = last_data + data
    data_len = len(data)
    if data_len == 0:
        break
    if data_len < 8:
        last_data = data
        continue
    msg_head = data[0:8]
    cur_pos = 8    
    msg_len, msg_id, seq = struct.unpack('=IHH', msg_head)

    if msg_len > data_len:
        last_data = data
        continue

    pb_len = msg_len - 8 - 16
    pb_data = data[cur_pos:cur_pos+pb_len]
    cur_pos = cur_pos + pb_len
    extern_data = data[cur_pos:cur_pos+16]
    cur_pos = cur_pos + 16
    player_id,t1,t1,t1 = struct.unpack('=QIHH', extern_data)
    if cur_pos > data_len:
        print 'err, cur_pos[%d] > data_len[%d]' %(cur_pos, data_len)
        break
    if cur_pos == data_len:
        last_data = ''
    else:
        last_data = data[cur_pos:]
        
#    data_len = data_len - 8 - 16
#    msg_format = "=IHH" + str(data_len) + 'sQIHH' 
#    msg_len, msg_id, seq, pb_data, player_id, t1, t1, t1 = struct.unpack(msg_format, data)

    if not player_id in WATCH_PLAYER:
        continue;

    if msg_id == 10302:
        req = comm_message_pb2.BagGrid()
        req.ParseFromString(pb_data)       
        print "update grid, pos:%u, id:%u, num:%u" % (req.index, req.id, req.num)

'''
    
