#ifndef _CONN_NODE_LOGIN_H__
#define _CONN_NODE_LOGIN_H__

#include "conn_node.h"

enum LOGIN_SERVER_STATE
{
//	LOGIN_SERVER_STATE_INVALID,  //不可用
	LOGIN_SERVER_STATE_NORMAL,  //可用
	LOGIN_SERVER_STATE_LOCK,  //锁定
};

#define MAX_LOGIN_SERVER_NUM 10

class conn_node_login: public conn_node_base
{
public:
	conn_node_login();
	virtual ~conn_node_login();

	virtual int recv_func(evutil_socket_t fd);
	virtual int send_one_msg(PROTO_HEAD *head, uint8_t force);
	void setid(int id);
	static conn_node_login *server_node[MAX_LOGIN_SERVER_NUM];
	static conn_node_login *get_normal_node();
	LOGIN_SERVER_STATE state;

	void locker();
	void unlocker();
	
private:
	int m_id;
	int transfer_to_client();
	int transfer_to_gameserver();
	int transfer_to_guildserver();	
	int transfer_to_loggersrv();
//	int broadcast_to_client();
	int kick_role(int fd);
	int handle_client_login(EXTERN_DATA *extern_data);
	int handle_client_enter(EXTERN_DATA* extern_data);
	int handle_client_login_success(EXTERN_DATA *extern_data, uint16_t seq);
	int handle_client_login_fail(EXTERN_DATA *extern_data, int result, uint16_t seq);
//	int handle_tiren();
	
};

#endif
