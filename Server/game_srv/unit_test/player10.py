from ctypes import *;
from _ctypes import *;
import move_pb2

loopcount = 0
playerid_base = 40000
api = CDLL('./libgame_srv.so')

def time100():
    global loopcount
    loopcount = loopcount + 1
    api.log(str(loopcount))
    if loopcount > 2:
        global loopcount
        global playerid_base
        global api
        loopcount = 0;
        api.log(str(loopcount))        
        playerid_base = playerid_base + 1
        api.log(str(playerid_base))        
        move_pos = [209, 43,
                    195, 52,
                    185, 59,
                    181, 87,
                    181, 103,
                    187, 116,
                    189, 130,
                    198, 134]
        request = move_pb2.move_request()
        api.log('11')                        
        for i in range(0, len(move_pos), 2):
            pos = request.data.add()
            pos.pos_x = move_pos[i]
            pos.pos_z = move_pos[i + 1]
        data = request.SerializeToString()
        api.log('22')                                
        a = c_char_p(data)
        b = len(data)
        api.log(str(b))        
        api.test_move_request_v2(playerid_base, a, b)
        api.log('aaaa')                
        
pDownTextInfoFv = CFUNCTYPE(c_void_p)
func2 = pDownTextInfoFv(time100)    

def py_install( ):
    try:
        playerid_base = 40000
        api.register_timer100_func(func2);        
    except Exception, e:
        print 'err'
        
#    api.log('dlclose so')    
#    dlclose(api._handle)
#    api = None
