from ctypes import *;
from _ctypes import *;



def py_install( ):
    api = CDLL('./libgame_srv.so')        
    api.log('1111111');
    api.test_create_monster(1, 10000, c_float(10.1111), c_float(20.324212));
    dlclose(api._handle)
    api = None

#pDownTextInfoFv = CFUNCTYPE(c_void_p, c_int, c_ulong)
#pDownTextInfoHandle = pDownTextInfoFv(py_event1);
#api.register_event(1, pDownTextInfoHandle);



