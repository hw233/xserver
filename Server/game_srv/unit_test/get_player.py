from ctypes import *;
from _ctypes import *;



def py_install( ):
    api = CDLL('./libgame_srv.so')        
    try:
        for i in range(40000, 40010):
            api.test_create_tmp_player(i);

        for i in range(40000, 40013):
            ret = c_void_p()
            api.log(str(byref(ret)))
            api.get_player(i, byref(ret))
            if ret.value:
                api.log(hex(ret.value))
                playerid = c_uint64()
                name = c_char_p()
                api.get_player_id_and_name(ret, byref(name), byref(playerid))
                api.log('id = ' + str(playerid.value) + ' name = ' + name.value)
            else:
                api.log('(nil)')
    except Exception, e:
        print 'err'
        
    api.log('dlclose so')    
    dlclose(api._handle)
    api = None
