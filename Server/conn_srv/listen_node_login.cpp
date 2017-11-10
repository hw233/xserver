#include "listen_node_login.h"
#include "game_event.h"
#include <assert.h>

listen_node_login::listen_node_login()
{
}

listen_node_login::~listen_node_login()
{
}

int listen_node_login::listen_pre_func()
{

	for (int i = 0; i < MAX_LOGIN_SERVER_NUM; ++i)
	{
		if (conn_node_login::server_node[i] == NULL)
			return (0);
	}
		
	LOG_ERR("%s %d: only %d server can connect", __FUNCTION__, __LINE__, MAX_LOGIN_SERVER_NUM);
	return (-1);
}

int listen_node_login::listen_after_func(evutil_socket_t fd)
{
	return (0);		
}

conn_node_base * listen_node_login::get_conn_node(evutil_socket_t fd)
{
	conn_node_login *ret = new conn_node_login();
	ret->fd = fd;

	for (int i = 0; i < MAX_LOGIN_SERVER_NUM; ++i)
	{
		if (conn_node_login::server_node[i] == NULL)
		{
			conn_node_login::server_node[i] = ret;
			ret->setid(i);
			ret->state = LOGIN_SERVER_STATE_NORMAL;
			return ret;
		}
	}
	assert(0);
	exit(0);
	return NULL;
}

