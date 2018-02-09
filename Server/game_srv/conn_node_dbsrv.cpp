#include "conn_node_dbsrv.h"
#include "game_event.h"
#include "conn_node_gamesrv.h"
#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>

conn_node_dbsrv *conn_node_dbsrv::connecter = NULL;
//char conn_node_dbsrv::send_buf[1024];

extern uint64_t  sg_server_id;

#define PROTO_STATIS_INFO_COMMIT2(magicId, num1, num2, num3, num4)     \
	do {  \
	PROTO_STATIS_INFO* proto = (PROTO_STATIS_INFO*)get_send_buf(SERVER_PROTO_STATIS_INFO, 0);  \
	proto_statis_info__init(proto, COMMIT_MYSQL, GAME_APPID, extern_data->open_id, extern_data->player_id, sg_server_id, (uint32_t)time(NULL)  \
	, magicId, num1, num2, num3, num4);  \
	proto->head.len = htons(sizeof(PROTO_STATIS_INFO)); \
	add_extern_data((PROTO_HEAD*)proto, extern_data);  \
	int result  = conn_node_gamesrv::connecter.send_one_msg((PROTO_HEAD*)proto, 1); \
	if (result != htons(proto->head.len)) {  \
	LOG_ERR("%s: send to client failed err[%d]", __FUNCTION__, errno); \
	}  \
	} while(false)  \

conn_node_dbsrv::conn_node_dbsrv()
{
	max_buf_len = 1024 * 1024;
	buf = (uint8_t *)malloc(max_buf_len + sizeof(EXTERN_DATA));
	assert(buf);
}

conn_node_dbsrv::~conn_node_dbsrv()
{
}

typedef int (*db_recv_func)(evutil_socket_t fd, conn_node_dbsrv *node);
extern db_recv_func g_db_recv_func;
int conn_node_dbsrv::recv_func(evutil_socket_t fd)
{
	if (g_db_recv_func)
		return g_db_recv_func(fd, this);
	return (0);
}

int conn_node_dbsrv::transfer_to_connsrv(void)
{
	int ret = 0;
	PROTO_HEAD *head = get_head();
	if (conn_node_gamesrv::connecter.send_one_msg(head, 1) != (int)ENDION_FUNC_4(head->len))
	{
		LOG_ERR("[%s:%d] send to connsrv failed err[%d]", __FUNCTION__, __LINE__, errno);
		ret = -2;
		goto done;
	}

done:
	return (ret);
}
