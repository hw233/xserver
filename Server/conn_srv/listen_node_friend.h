#ifndef LISTEN_NODE_FRIEND_H
#define LISTEN_NODE_FRIEND_H

#include "listen_node.h"
#include "conn_node_friend.h"

class listen_node_friend: public listen_node_base
{
public:
	listen_node_friend();
	~listen_node_friend();
	virtual int listen_pre_func();
	virtual int listen_after_func(evutil_socket_t fd);
	virtual conn_node_base * get_conn_node(evutil_socket_t fd);
};



#endif /* LISTEN_NODE_FRIEND_H */
