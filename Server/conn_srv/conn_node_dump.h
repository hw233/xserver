#ifndef _CONN_NODE_DUMP_H__
#define _CONN_NODE_DUMP_H__

#include "conn_node.h"

class conn_node_dump: public conn_node_base
{
public:
	conn_node_dump();
	virtual ~conn_node_dump();

	virtual int recv_func(evutil_socket_t fd);
	static conn_node_dump *server_node;
	static uint64_t player_id;
private:
	int transfer_to_client();
	int transfer_to_mailsrv();	
	int broadcast_to_all_client();	
	int broadcast_to_client();
	int kick_role(EXTERN_DATA *extern_data);	
	int kick_answer();
};

#endif
