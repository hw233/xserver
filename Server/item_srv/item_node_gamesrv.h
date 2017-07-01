#ifndef _CONN_NODE_SERVER_H__
#define _CONN_NODE_SERVER_H__

#include "conn_node.h"

class item_node_gamesrv: public conn_node_base
{
public:
	item_node_gamesrv();
	virtual ~item_node_gamesrv();

	virtual int recv_func(evutil_socket_t fd);
	static int add_cached_buf(PROTO_HEAD *head);
	static int send_all_cached_buf();	
	static item_node_gamesrv *server_node;
private:
	int transfer_to_client();

	static char cached_buf[1024 * 1024 * 1024];  //最多缓存1G的数据
	static int cached_len;                       //已经缓存的数据长度
};

#endif
