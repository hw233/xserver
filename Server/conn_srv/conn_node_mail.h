#ifndef _CONN_NODE_MAIL_H__
#define _CONN_NODE_MAIL_H__

#include "conn_node.h"

class conn_node_mail: public conn_node_base
{
public:
	conn_node_mail();
	virtual ~conn_node_mail();

	virtual int recv_func(evutil_socket_t fd);
	static conn_node_mail *server_node;
private:
	int transfer_to_client();
	int transfer_to_gamesrv();
};

#endif
