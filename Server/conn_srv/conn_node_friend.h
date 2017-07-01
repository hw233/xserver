#ifndef CONN_NODE_FRIEND_H
#define CONN_NODE_FRIEND_H

#include "conn_node.h"


class conn_node_friend: public conn_node_base
{
public:
	conn_node_friend();
	virtual ~conn_node_friend();

	virtual int recv_func(evutil_socket_t fd);
	virtual int send_one_msg(PROTO_HEAD *head, uint8_t force);
	static conn_node_friend *server_node;
	
private:
	int transfer_to_client();
	int broadcast_to_all_client();
	int transfer_to_gameserver();
	int transfer_to_mailsrv();
	int friend_to_gameserver(PROTO_HEAD *head);
	int broadcast_to_client();
};

#endif /* CONN_NODE_FRIEND_H */

