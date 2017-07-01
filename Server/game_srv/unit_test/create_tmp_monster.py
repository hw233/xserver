from ctypes import *;
from _ctypes import *;



def py_install( ):
    api = CDLL('./libgame_srv.so')
    pos_x = c_double(156.0)
    pos_z = c_double(141.0)    
    api.test_create_monster(151000001, 10000, pos_x, pos_z);
    dlclose(api._handle)
    api = None

#pDownTextInfoFv = CFUNCTYPE(c_void_p, c_int, c_ulong)
#pDownTextInfoHandle = pDownTextInfoFv(py_event1);
#api.register_event(1, pDownTextInfoHandle);



