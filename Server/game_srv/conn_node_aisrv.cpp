#include "conn_node_aisrv.h"
#include "game_event.h"
#include "conn_node_gamesrv.h"
#include "game_srv.h"
#include "lua_config.h"
#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>

conn_node_aisrv *conn_node_aisrv::server_node = NULL;
bool conn_node_aisrv::connected = false;
//char conn_node_aisrv::send_buf[1024];

conn_node_aisrv::conn_node_aisrv()
{
	max_buf_len = 1024 * 1024;
	buf = (uint8_t *)malloc(max_buf_len + sizeof(EXTERN_DATA));
	assert(buf);
}

conn_node_aisrv::~conn_node_aisrv()
{
	server_node = NULL;
	connected = false;
}

int conn_node_aisrv::recv_func(evutil_socket_t fd)
{
	if (g_ai_recv_func)
		return g_ai_recv_func(fd, this);
	return (0);
}

int conn_node_aisrv::try_connect_aisrv()
{
#ifdef USE_AISRV
	if (connected)
		return (0);
	if (sg_ai_srv_port <= 0)
		return (-1);

	if (!server_node)
	{
		server_node = new conn_node_aisrv;
	}
	if (!server_node)
	{
		return (-1);
	}

    struct sockaddr_in sin;	
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port   = htons(sg_ai_srv_port);
	int ret        = evutil_inet_pton(AF_INET, "127.0.0.1", &sin.sin_addr);
	if (ret != 1)
	{
//		LOG_ERR("%s %d: evutil_inet_pton failed[%d]", __FUNCTION__, __LINE__, ret);
		return -1;
	}

	ret = game_add_connect_event((struct sockaddr *)&sin, sizeof(sin), server_node);
	if (ret <= 0)
		return -2;
	connected = true;
#endif	
	return (0);
}

int send_to_aisrv(uint64_t player_id, uint16_t msg_id)
{
	EXTERN_DATA extern_data;
	extern_data.player_id = player_id;
	return fast_send_msg_base(conn_node_aisrv::server_node, &extern_data, msg_id, 0, 0);	
}

