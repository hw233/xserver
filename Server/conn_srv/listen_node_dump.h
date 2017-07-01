#ifndef LISTEN_NODE_DUMP_H__
#define LISTEN_NODE_DUMP_H__
#include "listen_node.h"
#include "conn_node_dump.h"

class listen_node_dump: public listen_node_base
{
public:
	listen_node_dump();
	~listen_node_dump();
	virtual int listen_pre_func();
	virtual int listen_after_func(evutil_socket_t fd);
	virtual conn_node_base * get_conn_node(evutil_socket_t fd);
};



#endif
