#ifndef _CONN_NODE_DUMPSRV_H__ 
#define _CONN_NODE_DUMPSRV_H__

#include "conn_node.h"
#include <string>


extern uint32_t sg_server_id;
extern uint32_t sg_max_conn;

extern std::string sg_server_key;
/// map<playerid, pair<day, ts> >
extern std::map<uint32_t, std::pair<uint32_t, uint32_t> > sg_filter_list;
extern long sg_filter_map_locker;


extern struct event sg_event_timer;
extern struct timeval sg_timeout;	
extern void cb_on_timer(evutil_socket_t, short, void *arg);


//extern std::set<uint32_t>  filterPlayerList;

class conn_node_dumpsrv: public conn_node_base
{
public:
	conn_node_dumpsrv();
	virtual ~conn_node_dumpsrv();

	virtual int recv_func(evutil_socket_t fd);
	
	static conn_node_dumpsrv connecter;
private:
};



#endif

