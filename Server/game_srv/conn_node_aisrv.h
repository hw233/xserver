#ifndef _CONN_NODE_AISRV_H__
#define _CONN_NODE_AISRV_H__

#include "conn_node.h"
#include "lua_config.h"
//处理和ai服务器连接的对象
class conn_node_aisrv: public conn_node_base
{
public:
	conn_node_aisrv();
	virtual ~conn_node_aisrv();

	virtual int recv_func(evutil_socket_t fd);
	
	static conn_node_aisrv *server_node;
	static bool connected;
	static int try_connect_aisrv();
//	static char send_buf[1024];

	int transfer_to_connsrv(void);

};

template<typename PACK_STRUCT>
int send_to_aisrv(uint64_t player_id, uint16_t msg_id, size_t(*func)(const PACK_STRUCT *, uint8_t *), PACK_STRUCT &obj)
{
	if (sg_ai_srv_port <= 0)
		return (-1);
	
	if (!conn_node_aisrv::server_node || !conn_node_aisrv::connected)
	{
		conn_node_aisrv::try_connect_aisrv();
	}

	if (!conn_node_aisrv::server_node || !conn_node_aisrv::connected)
	{
//		LOG_ERR("%s: can not connect aisrv", __FUNCTION__);
		return (-1);
	}	
	
	EXTERN_DATA extern_data;
	extern_data.player_id = player_id;
	size_t size = (*func)(&obj, conn_node_aisrv::server_node->get_send_data());
	return fast_send_msg_base(conn_node_aisrv::server_node, &extern_data, msg_id, size, 0);
}

int send_to_aisrv(uint64_t player_id, uint16_t msg_id);

#endif
