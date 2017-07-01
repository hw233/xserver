#ifndef _CONN_NODE_TEST_H__
#define _CONN_NODE_TEST_H__

#include "conn_node.h"

class conn_node_test: public conn_node_base
{
public:
	conn_node_test();
	virtual ~conn_node_test();

	virtual int recv_func(evutil_socket_t fd);

	static conn_node_test *conn_test;
};



#endif
