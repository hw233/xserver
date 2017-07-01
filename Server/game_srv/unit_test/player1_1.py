from ctypes import *;
from _ctypes import *;
import move_direct_pb2

def py_install( ):
    api = CDLL('./libgame_srv.so')
    try:
        playerid = 30001
        move_pos = [209, 43,
                    195, 52,
                    185, 59,
                    181, 87,
                    181, 103,
                    187, 116,
                    189, 130,
                    198, 134]
        request = move_direct_pb2.move_start_request()
        request.cur_pos.pos_x = 209
        request.cur_pos.pos_z = 43
        request.direct_x = 0;
        request.direct_y = 0;
        request.direct_z = 1;                

        data = request.SerializeToString()
        a = c_char_p(data)
        b = len(data)
        api.test_move_start_request(playerid, a, b)
    except Exception, e:
        print 'err'
        
    api.log('dlclose so')    
    dlclose(api._handle)
    api = None
