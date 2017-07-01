#include "listen_node_friend.h"

#include "game_event.h"
#include <assert.h>

listen_node_friend::listen_node_friend()
{
}

listen_node_friend::~listen_node_friend()
{
}

int listen_node_friend::listen_pre_func()
{
	if (conn_node_friend::server_node == NULL)
		return (0);
	LOG_ERR("%s %d: only one server can connect", __FUNCTION__, __LINE__);
	return (-1);
}

int listen_node_friend::listen_after_func(evutil_socket_t fd)
{
	return (0);		
}

conn_node_base * listen_node_friend::get_conn_node(evutil_socket_t fd)
{
	assert(conn_node_friend::server_node == NULL);
	conn_node_friend *ret = new conn_node_friend();
	ret->fd = fd;
	conn_node_friend::server_node = ret;
	return ret;
}

