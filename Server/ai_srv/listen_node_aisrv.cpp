#include "listen_node_aisrv.h"
#include <assert.h>
#include "game_event.h"

listen_node_aisrv::listen_node_aisrv()
{
}

listen_node_aisrv::~listen_node_aisrv()
{
}

int listen_node_aisrv::listen_pre_func()
{
    if (conn_node_aisrv::server_node == NULL)
        return (0);
    LOG_ERR("%s %d: only one server can connect", __PRETTY_FUNCTION__, __LINE__);
    return (-1);
}

int listen_node_aisrv::listen_after_func(evutil_socket_t fd)
{
    return (0);
}

conn_node_base *listen_node_aisrv::get_conn_node(evutil_socket_t fd)
{
    conn_node_aisrv *ret = new conn_node_aisrv();
    ret->fd              = fd;
    assert(conn_node_aisrv::server_node == NULL);
    conn_node_aisrv::server_node = ret;
    return ret;
}
