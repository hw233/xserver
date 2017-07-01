#ifndef LISTEN_NODE_SERVER_H__
#define LISTEN_NODE_SERVER_H__
#include "listen_node.h"
#include "item_node_gamesrv.h"

class listen_node_gamesrv: public listen_node_base
{
public:
	listen_node_gamesrv();
	~listen_node_gamesrv();
	virtual int listen_pre_func();
	virtual int listen_after_func(evutil_socket_t fd);
	virtual conn_node_base * get_conn_node(evutil_socket_t fd);
};



#endif
