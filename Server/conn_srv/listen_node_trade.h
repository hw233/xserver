#ifndef LISTEN_NODE_TRADE_H__
#define LISTEN_NODE_TRADE_H__
#include "listen_node.h"
#include "conn_node_trade.h"

class listen_node_trade: public listen_node_base
{
public:
	listen_node_trade();
	~listen_node_trade();
	virtual int listen_pre_func();
	virtual int listen_after_func(evutil_socket_t fd);
	virtual conn_node_base * get_conn_node(evutil_socket_t fd);
};



#endif
