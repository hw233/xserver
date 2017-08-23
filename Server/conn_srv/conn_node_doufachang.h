#ifndef _CONN_NODE_DOUFACHANG_H__
#define _CONN_NODE_DOUFACHANG_H__

#include "conn_node.h"

class conn_node_doufachang: public conn_node_base
{
public:
	conn_node_doufachang();
	virtual ~conn_node_doufachang();

	virtual int recv_func(evutil_socket_t fd);
	static conn_node_doufachang *server_node;
private:
	int transfer_to_client();
	int transfer_to_gameserver();	
};

#endif
