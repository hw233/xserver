#ifndef LISTEN_NODE_GUILD_H__
#define LISTEN_NODE_GUILD_H__
#include "listen_node.h"
#include "conn_node_guild.h"

class listen_node_guild: public listen_node_base
{
public:
	listen_node_guild();
	~listen_node_guild();
	virtual int listen_pre_func();
	virtual int listen_after_func(evutil_socket_t fd);
	virtual conn_node_base * get_conn_node(evutil_socket_t fd);
};



#endif
