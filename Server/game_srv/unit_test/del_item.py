from ctypes import *;
from _ctypes import *;



def py_install( ):
    api = CDLL('./libgame_srv.so')        
    api.del_item(c_ulong(12884902789), 201020001, 1)
    dlclose(api._handle)
    api = None

#pDownTextInfoFv = CFUNCTYPE(c_void_p, c_int, c_ulong)
#pDownTextInfoHandle = pDownTextInfoFv(py_event1);
#api.register_event(1, pDownTextInfoHandle);



