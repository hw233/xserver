from ctypes import *;
from _ctypes import *;
import move_pb2


def py_install( ):
    api = CDLL('./libgame_srv.so')        
    try:
        playerid = 30000
        move_pos = [10.111111,10.111111, 20.22222222,20.222222, 30,30]
        request = move_pb2.move_request()
        for i in range(0, len(move_pos), 2):
            pos = request.data.add()
            pos.pos_x = move_pos[i]
            pos.pos_z = move_pos[i + 1]
        data = request.SerializeToString()
        a = c_char_p(data)
        b = len(data)
        api.test_move_request_v2(playerid, a, b)
    except Exception, e:
        print 'err'
        
    api.log('dlclose so')    
    dlclose(api._handle)
    api = None
