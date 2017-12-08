#include "ai_srv.h"
#include "conn_node_aisrv.h"
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string>
#include "../proto/msgid.h"
#include "error_code.h"
#include "game_event.h"

#define UNUSED(x) (void)(x)

conn_node_aisrv::conn_node_aisrv()
{
    max_buf_len = 1024 * 1024;
    buf         = (uint8_t *)malloc(max_buf_len + sizeof(EXTERN_DATA));
    assert(buf);
}

conn_node_aisrv::~conn_node_aisrv()
{
    server_node = NULL;
}

int conn_node_aisrv::recv_func(evutil_socket_t fd)
{
	if (g_ai_recv_func)
		return g_ai_recv_func(fd, this);
	return (0);
    // EXTERN_DATA *extern_data;
    // UNUSED(extern_data);
    // PROTO_HEAD *head;
    // for (;;)
    // {
    //     int ret = get_one_buf();
    //     if (ret == 0)
    //     {
    //         head        = (PROTO_HEAD *)buf_head();
    //         extern_data = get_extern_data(head);
    //         switch (ENDION_FUNC_2(head->msg_id))
    //         {
    //             default:
    //                 break;
    //         }
    //     }

    //     if (ret < 0)
    //     {
    //         LOG_INFO("%s %d: connect closed from fd %u, err = %d", __FUNCTION__, __LINE__, fd, errno);
    //         exit(0);
    //         return (-1);
    //     }
    //     else if (ret > 0)
    //     {
    //         break;
    //     }

    //     ret = remove_one_buf();
    // }
    // return (0);
}

int send_to_gamesrv(uint64_t player_id, uint16_t msg_id)
{
	EXTERN_DATA extern_data;
	extern_data.player_id = player_id;
	return fast_send_msg_base(conn_node_aisrv::server_node, &extern_data, msg_id, 0, 0);	
}

conn_node_aisrv *conn_node_aisrv::server_node;
