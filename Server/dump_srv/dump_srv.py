#!/usr/bin/python

import sys
from socket import *
import struct
import move_pb2
import move_direct_pb2

WATCH_PLAYER = {12884902615, 4294967949, 12884902629}

HOST='127.0.0.1'
PORT=10697
BUFSIZ=1024
ADDR=(HOST, PORT)
client=socket(AF_INET, SOCK_STREAM)
client.connect(ADDR)

last_data = ""
player_list = {}

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

#    if not player_id in WATCH_PLAYER:
#        continue;

    if msg_id == 10103:
        req = move_pb2.sight_changed_notify()
        req.ParseFromString(pb_data)       
#        print "len = %d, id = %d, seq = %d, player_id = %d add %d delete %d" %(msg_len, msg_id, seq, player_id, len(req.add_player), len(req.delete_player))        
        
        for t1 in req.add_player:
            if len(t1.data) == 0:
                print 'err add player %lu data len == 0' %t1.playerid
                sys.exit(0)                

            print "add player %lu %s %.1f %.1f" % (t1.playerid, t1.name, t1.data[0].pos_x, t1.data[0].pos_z)
            if t1.playerid in player_list:
                print 'err add player %lu' %t1.playerid
                sys.exit(0)
            player_list[t1.playerid] = t1.name
        for t1 in req.delete_player:            
            if not t1 in player_list:
                print 'err del player %lu' %t1
                sys.exit(0)
            print "del player %lu %s" % (t1, player_list[t1])
            del player_list[t1]

#    if msg_id == 10102:  #MSG_ID_MOVE_NOTIFY
#        req = move_pb2.move_notify()
#        req.ParseFromString(pb_data)
#        print "get move notify from player %lu " % req.playerid
#        if req.playerid == 0:
#            print 'player id == 0'
#            break;
        
#    print "len = %d, id = %d, seq = %d, player_id = %d" %(msg_len, msg_id, seq, player_id)

    if msg_id == 10100:
#        print "len = %d, id = %d, seq = %d, player_id = %d" %(msg_len, msg_id, seq, player_id)        
        req = move_pb2.move_request()
        req.ParseFromString(pb_data)
        for t1 in req.data:
            print "%.1f : %.1f" %(t1.pos_x,t1.pos_z)
#            
#    if msg_id == 10105:
#        print "len = %d, id = %d, seq = %d, player_id = %d" %(msg_len, msg_id, seq, player_id)
#        req = move_direct_pb2.move_start_request()
#        req.ParseFromString(pb_data)
#        print "%.1f : %.1f | %.1f %.1f %.1f" %(req.cur_pos.pos_x, req.cur_pos.pos_z, req.direct_x, req.direct_z, req.direct_y)
#        
#    if msg_id == 10108:
#        print "len = %d, id = %d, seq = %d, player_id = %d" %(msg_len, msg_id, seq, player_id)        
#        req = move_direct_pb2.move_stop_request()
#        req.ParseFromString(pb_data)
#        print "%.1f : %.1f" %(req.cur_pos.pos_x, req.cur_pos.pos_z)
        



    
