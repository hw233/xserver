#include "listen_node_doufachang.h"
#include "game_event.h"
#include <assert.h>
#include <netinet/tcp.h>

listen_node_doufachang::listen_node_doufachang()
{
}

listen_node_doufachang::~listen_node_doufachang()
{
}

int listen_node_doufachang::listen_pre_func()
{
	if (conn_node_doufachang::server_node == NULL)
		return (0);
	LOG_ERR("%s %d: only one server can connect", __FUNCTION__, __LINE__);
	return (-1);
}

int listen_node_doufachang::listen_after_func(evutil_socket_t fd)
{
	return (0);		
}

conn_node_base * listen_node_doufachang::get_conn_node(evutil_socket_t fd)
{
	conn_node_doufachang *ret = new conn_node_doufachang();
	ret->fd = fd;
    int flag = 0; 	
    setsockopt(fd, IPPROTO_TCP, TCP_CORK, (char *)&flag, sizeof(flag));
	assert(conn_node_doufachang::server_node == NULL);
	conn_node_doufachang::server_node = ret;
	return ret;
}

