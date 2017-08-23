#ifndef _CONN_NODE_RANKSRV_H__ 
#define _CONN_NODE_RANKSRV_H__

#include "conn_node.h"

class conn_node_ranksrv: public conn_node_base
{
	typedef int (conn_node_ranksrv::*handle_func)(EXTERN_DATA*);
	typedef std::map<uint32_t, handle_func> HandleMap;
public:
	conn_node_ranksrv();
	virtual ~conn_node_ranksrv();

	void add_msg_handle(uint32_t msg_id, handle_func func);

	virtual int recv_func(evutil_socket_t fd);

	static conn_node_ranksrv connecter;
private:
	int handle_refresh_player_info(EXTERN_DATA *extern_data);
	int handle_rank_info_request(EXTERN_DATA *extern_data);
	int handle_player_online_notify(EXTERN_DATA *extern_data);
	int handle_refresh_player_word_boss_info(EXTERN_DATA *extern_data);

private:
	HandleMap   m_handleMap;
};



#endif

