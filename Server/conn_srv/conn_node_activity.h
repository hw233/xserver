#ifndef _CONN_NODE_ACTIVITY_H__
#define _CONN_NODE_ACTIVITY_H__

#include "conn_node.h"

class conn_node_activity: public conn_node_base
{
public:
	conn_node_activity();
	virtual ~conn_node_activity();

	virtual int recv_func(evutil_socket_t fd);
	static conn_node_activity *server_node;
private:
	int dispatch_message();
	int transfer_to_client();
	int transfer_to_gamesrv();
	int transfer_to_mailsrv();
	int transfer_to_ranksrv();	
	int transfer_to_guildsrv();
	int broadcast_to_client();
	int broadcast_to_all_client();
};

#endif
