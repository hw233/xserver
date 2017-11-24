#include "listen_node_raidsrv.h"
#include "game_event.h"
#include <assert.h>

listen_node_raidsrv::listen_node_raidsrv()
{
}

listen_node_raidsrv::~listen_node_raidsrv()
{
}

int listen_node_raidsrv::listen_pre_func()
{
//	if (conn_node_gamesrv::server_node == NULL)
//		return (0);
//	LOG_ERR("%s %d: only one server can connect", __PRETTY_FUNCTION__, __LINE__);
	return (0);
}

int listen_node_raidsrv::listen_after_func(evutil_socket_t fd)
{
	return (0);		
}

conn_node_base * listen_node_raidsrv::get_conn_node(evutil_socket_t fd)
{
	conn_node_raidsrv *ret = new conn_node_raidsrv();
	ret->fd = fd;
	return ret;
}

