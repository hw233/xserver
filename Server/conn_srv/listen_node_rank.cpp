#include "listen_node_rank.h"
#include "game_event.h"
#include <assert.h>
#include <netinet/tcp.h>

listen_node_rank::listen_node_rank()
{
}

listen_node_rank::~listen_node_rank()
{
}

int listen_node_rank::listen_pre_func()
{
	if (conn_node_rank::server_node == NULL)
		return (0);
	LOG_ERR("%s %d: only one server can connect", __PRETTY_FUNCTION__, __LINE__);
	return (-1);
}

int listen_node_rank::listen_after_func(evutil_socket_t fd)
{
	return (0);		
}

conn_node_base * listen_node_rank::get_conn_node(evutil_socket_t fd)
{
	conn_node_rank *ret = new conn_node_rank();
	ret->fd = fd;
    int flag = 0; 	
    setsockopt(fd, IPPROTO_TCP, TCP_CORK, (char *)&flag, sizeof(flag));
	assert(conn_node_rank::server_node == NULL);
	conn_node_rank::server_node = ret;
	return ret;
}

