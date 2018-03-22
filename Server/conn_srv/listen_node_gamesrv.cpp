#include "listen_node_gamesrv.h"
#include "game_event.h"
#include <assert.h>

listen_node_gamesrv::listen_node_gamesrv()
{
}

listen_node_gamesrv::~listen_node_gamesrv()
{
}

int listen_node_gamesrv::listen_pre_func()
{
	if (conn_node_gamesrv::server_node == NULL)
		return (0);
	LOG_ERR("%s %d: only one server can connect", __PRETTY_FUNCTION__, __LINE__);
	return (-1);
}

int listen_node_gamesrv::listen_after_func(evutil_socket_t fd)
{
	return (0);		
}

extern void on_gamesrv_write(int fd, short ev, void *arg);
conn_node_base * listen_node_gamesrv::get_conn_node(evutil_socket_t fd)
{
	conn_node_gamesrv *ret = new conn_node_gamesrv();
	ret->fd = fd;
	assert(conn_node_gamesrv::server_node == NULL);
	conn_node_gamesrv::server_node = ret;

	/* 创建写事件，但是在我们有数据可写之前，不要添加它。 */
//	event_set(&ret->ev_write, fd, EV_WRITE, on_write, ret);
	if (0 != event_assign(&ret->ev_write, base, fd, EV_WRITE, on_gamesrv_write, (void *)ret)) {
		LOG_ERR("%s: event_new failed[%d]", __FUNCTION__, errno);
		delete ret;
		return NULL;
	}
	
	if (conn_node_gamesrv::send_all_cached_buf() != 0) {
		delete ret;
		conn_node_gamesrv::server_node = NULL;
		return NULL;
	}
	return ret;
}

