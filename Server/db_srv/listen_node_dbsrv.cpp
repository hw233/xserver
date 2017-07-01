#include "listen_node_dbsrv.h"
#include "game_event.h"
#include <assert.h>

listen_node_dbsrv::listen_node_dbsrv()
{
}

listen_node_dbsrv::~listen_node_dbsrv()
{
}

int listen_node_dbsrv::listen_pre_func()
{
	if (conn_node_dbsrv::server_node == NULL)
		return (0);
	LOG_ERR("%s %d: only one server can connect", __FUNCTION__, __LINE__);
	return (-1);
}

int listen_node_dbsrv::listen_after_func(evutil_socket_t fd)
{
	return (0);		
}

conn_node_base * listen_node_dbsrv::get_conn_node(evutil_socket_t fd)
{
	conn_node_dbsrv *ret = new conn_node_dbsrv();
	ret->fd = fd;
	assert(conn_node_dbsrv::server_node == NULL);
	conn_node_dbsrv::server_node = ret;
	return ret;
}

