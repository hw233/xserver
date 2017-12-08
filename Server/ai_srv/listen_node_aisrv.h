#ifndef LISTEN_NODE_AISRV_H__
#define LISTEN_NODE_AISRV_H__
#include "conn_node_aisrv.h"
#include "listen_node.h"

class listen_node_aisrv : public listen_node_base
{
public:
    listen_node_aisrv();
    ~listen_node_aisrv();
    virtual int listen_pre_func();
    virtual int listen_after_func(evutil_socket_t fd);
    virtual conn_node_base* get_conn_node(evutil_socket_t fd);
};



#endif
