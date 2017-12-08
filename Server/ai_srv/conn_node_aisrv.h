#ifndef _CONN_NODE_AISRV_H__
#define _CONN_NODE_AISRV_H__

#include "conn_node.h"

class conn_node_aisrv : public conn_node_base
{
public:
    conn_node_aisrv();
    virtual ~conn_node_aisrv();

    virtual int recv_func(evutil_socket_t fd);
    static conn_node_aisrv *server_node;

private:
};

template<typename PACK_STRUCT>
int send_to_gamesrv(uint64_t player_id, uint16_t msg_id, size_t(*func)(const PACK_STRUCT *, uint8_t *), PACK_STRUCT &obj)
{
	EXTERN_DATA extern_data;
	extern_data.player_id = player_id;
	size_t size = (*func)(&obj, conn_node_aisrv::server_node->get_send_data());
	return fast_send_msg_base(conn_node_aisrv::server_node, &extern_data, msg_id, size, 0);
}

int send_to_gamesrv(uint64_t player_id, uint16_t msg_id);

#endif
