from ctypes import *;
from _ctypes import *;



def py_install( ):
    api = CDLL('./libgame_srv.so')        
    for id in range(201020001, 201020006):
        api.add_item(c_ulong(12884902789), id, 2)
    dlclose(api._handle)
    api = None

#pDownTextInfoFv = CFUNCTYPE(c_void_p, c_int, c_ulong)
#pDownTextInfoHandle = pDownTextInfoFv(py_event1);
#api.register_event(1, pDownTextInfoHandle);



