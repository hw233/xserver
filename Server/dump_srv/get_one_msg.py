#!/usr/bin/python
# coding: UTF-8

import sys
from socket import *
import struct
import re

BUFSIZ=1024

def get_dumpsrv_port():
    pattern = re.compile(r'(^[ 	]*conn_srv_dump_port[ 	]*=[ 	]*)([0-9]*)(.*$)')
    f = open('../server_info.ini')
    line = f.readline()
    while line:
        match = pattern.match(line)
        if match:
            a = match.groups()
            if len(a) == 3:
                f.close()
                return int(a[1])
        line = f.readline()


#返回0表示了数据没读完
#返回-1表示网络断开
#返回1表示读到一条数据
def get_one_msg(client, last_data):
    data_len = len(last_data)
    if data_len >= 8:
        msg_head = last_data[0:8]
        msg_len, msg_id, seq = struct.unpack('=IHH', msg_head)
        if msg_len > data_len:        
            data=client.recv(BUFSIZ)
            data = last_data + data
        else:
            data = last_data
    else:
        data=client.recv(BUFSIZ)
        data = last_data + data
        
    data_len = len(data)
    if data_len == 0:
        return -1, 0,0,0,0
    if data_len < 8:
        last_data = data
        return 0, last_data, 0,0,0
    msg_head = data[0:8]
    cur_pos = 8    
    msg_len, msg_id, seq = struct.unpack('=IHH', msg_head)

    if msg_len > data_len:
        last_data = data
        return 0, last_data, 0,0,0

    pb_len = msg_len - 8 - 16
    pb_data = data[cur_pos:cur_pos+pb_len]
    cur_pos = cur_pos + pb_len
    extern_data = data[cur_pos:cur_pos+16]
    cur_pos = cur_pos + 16
    player_id,t1,t1,t1 = struct.unpack('=QIHH', extern_data)
    if cur_pos > data_len:
        print 'err, cur_pos[%d] > data_len[%d]' %(cur_pos, data_len)
        return -1, 0,0,0,0
    if cur_pos == data_len:
        last_data = ''
    else:
        last_data = data[cur_pos:]
    return 1, last_data, player_id, msg_id, pb_data
    
