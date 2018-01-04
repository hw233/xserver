#ifndef _CONN_NODE_RANK_H__
#define _CONN_NODE_RANK_H__

#include "conn_node.h"

class conn_node_rank: public conn_node_base
{
public:
	conn_node_rank();
	virtual ~conn_node_rank();

	virtual int recv_func(evutil_socket_t fd);
	static conn_node_rank *server_node;
private:
	int dispatch_message();
	int transfer_to_client();
	int transfer_to_gamesrv();
	int transfer_to_friendsrv();
	int transfer_to_mailsrv();
	int broadcast_to_client();
};

#endif
