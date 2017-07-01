from ctypes import *;
from _ctypes import *;



def py_install( ):
    api = CDLL('./libgame_srv.so')        
    api.test3('1111111');
    player_id = []
    base = 40001
    while base < 40002:
        player_id.append(base)
        base = base + 1
    
    for iter in player_id:
        api.test_create_tmp_player(iter);
    dlclose(api._handle)
    api = None

#pDownTextInfoFv = CFUNCTYPE(c_void_p, c_int, c_ulong)
#pDownTextInfoHandle = pDownTextInfoFv(py_event1);
#api.register_event(1, pDownTextInfoHandle);



