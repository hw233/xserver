from ctypes import *;

def py_event1(event_id, player):
    print "in py_event1, id =", event_id
    age = api.get_age(player)
    print "age =", age

def py_install( ):
    api = CDLL('./libgame_srv.so')
#    print "py_install"
    api.test1();
    api.test2(100);
    api.test3('py_install test string');
    player_id = 30000
    pos_array = c_double * 10
    pos_x = pos_array(1.123,2.34334,3.435435,4,5)
    pos_z = pos_array(10,20,30,40,50)
    api.test_move_request(player_id, 8, pos_x, pos_z)
    dlclose(api._handle)
    api = None
   
#pDownTextInfoFv = CFUNCTYPE(c_void_p, c_int, c_ulong)
#pDownTextInfoHandle = pDownTextInfoFv(py_event1);
#api.register_event(1, pDownTextInfoHandle);



