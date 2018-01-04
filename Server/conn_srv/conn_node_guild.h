#ifndef _CONN_NODE_GUILD_H__
#define _CONN_NODE_GUILD_H__

#include "conn_node.h"

class conn_node_guild: public conn_node_base
{
public:
	conn_node_guild();
	virtual ~conn_node_guild();

	virtual int recv_func(evutil_socket_t fd);
	static conn_node_guild *server_node;
private:
	int transfer_to_client();
	int transfer_to_gamesrv();
	int transfer_to_mailsrv();
	int transfer_to_ranksrv();	
	int transfer_to_activitysrv();	
	int broadcast_to_client();
};

#endif
