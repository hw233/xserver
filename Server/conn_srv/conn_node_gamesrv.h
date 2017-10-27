#ifndef _CONN_NODE_SERVER_H__
#define _CONN_NODE_SERVER_H__

#include "conn_node.h"

class conn_node_gamesrv: public conn_node_base
{
public:
	conn_node_gamesrv();
	virtual ~conn_node_gamesrv();

	virtual int recv_func(evutil_socket_t fd);
	static int add_cached_buf(PROTO_HEAD *head);
	static int send_all_cached_buf();	
	static conn_node_gamesrv *server_node;
private:
	int dispatch_message();
	int transfer_to_client();
	int transfer_to_friendsrv();
	int game_to_friendsrv();	
	int transfer_to_mailsrv();	
	int transfer_to_guildsrv();
	int transfer_to_doufachang();
	int transfer_to_ranksrv();	
	int transfer_to_tradesrv();	
	int broadcast_to_all_client();	
	int broadcast_to_client();
	int kick_role(EXTERN_DATA *extern_data);	
	int kick_answer();

	static char cached_buf[1024 * 1024 * 1024];  //最多缓存1G的数据
	static int cached_len;                       //已经缓存的数据长度
};

#endif
