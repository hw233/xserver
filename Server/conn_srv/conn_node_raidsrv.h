#ifndef _CONN_NODE_RAIDSERVER_H__
#define _CONN_NODE_RAIDSERVER_H__

#include "conn_node.h"

#define MAX_RAIDSRV (10)

class conn_node_raidsrv: public conn_node_base
{
public:
	conn_node_raidsrv();
	virtual ~conn_node_raidsrv();

	virtual int recv_func(evutil_socket_t fd);
	static conn_node_raidsrv *server_node[MAX_RAIDSRV];
private:
	int dispatch_message();
	int transfer_to_client();
	int transfer_to_friendsrv();
	int game_to_friendsrv();	
	int transfer_to_mailsrv();
	int transfer_to_gamesrv();	
	int transfer_to_guildsrv();
	int transfer_to_doufachang();
	int transfer_to_ranksrv();	
	int transfer_to_tradesrv();	
	int broadcast_to_all_client();	
	int broadcast_to_client();
};

#endif
