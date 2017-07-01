from ctypes import *;
from _ctypes import *;

api = CDLL('./libgame_srv.so')
def time10():
    api.log('time10')

def time100():
    api.log('time100')    

pDownTextInfoFv = CFUNCTYPE(c_void_p)
func = pDownTextInfoFv(time10)
func2 = pDownTextInfoFv(time100)    
    
def py_install( ):
    api.register_timer10_func(func);
    api.register_timer100_func(func2);
        
#    api.log('dlclose so')    
#    dlclose(api._handle)
#    api = None        
