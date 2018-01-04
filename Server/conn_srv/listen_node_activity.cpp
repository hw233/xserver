#include "listen_node_activity.h"
#include "game_event.h"
#include <assert.h>
#include <netinet/tcp.h>

listen_node_activity::listen_node_activity()
{
}

listen_node_activity::~listen_node_activity()
{
}

int listen_node_activity::listen_pre_func()
{
	if (conn_node_activity::server_node == NULL)
		return (0);
	LOG_ERR("%s %d: only one server can connect", __PRETTY_FUNCTION__, __LINE__);
	return (-1);
}

int listen_node_activity::listen_after_func(evutil_socket_t fd)
{
	return (0);		
}

conn_node_base * listen_node_activity::get_conn_node(evutil_socket_t fd)
{
	conn_node_activity *ret = new conn_node_activity();
	ret->fd = fd;
    int flag = 0; 	
    setsockopt(fd, IPPROTO_TCP, TCP_CORK, (char *)&flag, sizeof(flag));
	assert(conn_node_activity::server_node == NULL);
	conn_node_activity::server_node = ret;
	return ret;
}

