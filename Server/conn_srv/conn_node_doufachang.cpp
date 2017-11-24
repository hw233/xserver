#include "conn_node_doufachang.h"
#include "conn_node_client.h"
#include "conn_node_login.h"
#include "conn_node_gamesrv.h"
#include "conn_node_mail.h"
#include "game_event.h"
#include "flow_record.h"
#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include "msgid.h"

conn_node_doufachang::conn_node_doufachang()
{
	max_buf_len = 1024 * 1024;
	buf = (uint8_t *)malloc(max_buf_len + sizeof(EXTERN_DATA));
	assert(buf);	
}

conn_node_doufachang::~conn_node_doufachang()
{
	server_node = NULL;
}

int conn_node_doufachang::recv_func(evutil_socket_t fd)
{
	for (;;) {
		int ret = get_one_buf();
		if (ret == 0) {
			if (transfer_to_client() != 0) {
				LOG_INFO("%s %d: transfer to client failed", __FUNCTION__, __LINE__);
			}
		} else if (ret < 0) {
			LOG_INFO("%s %d: connect closed from fd %u, err = %d", __FUNCTION__, __LINE__, fd, errno);
			return (-1);		
		} else if (ret > 0) {
			break;
		}
		
		ret = remove_one_buf();
	}
	return (0);
}

int conn_node_doufachang::transfer_to_gameserver()
{
	int ret = 0;
	PROTO_HEAD *head;
	head = (PROTO_HEAD *)buf_head();

	if (!conn_node_gamesrv::server_node) {
		LOG_ERR("[%s:%d] do not have game server connected", __FUNCTION__, __LINE__);
		ret = -1;
		goto done;
	}

	if (conn_node_gamesrv::server_node->send_one_msg(head, 1) != (int)ENDION_FUNC_4(head->len)) {
		LOG_ERR("[%s:%d] send to gameserver failed err[%d]", __FUNCTION__, __LINE__, errno);
		ret = -2;
		goto done;
	}

#ifdef FLOW_MONITOR
	add_on_other_server_answer_msg(head);
#endif

done:	
	return (ret);
}

int conn_node_doufachang::transfer_to_client()
{
	uint32_t old_len;
	PROTO_HEAD *head;
	EXTERN_DATA *extern_data;
	conn_node_client *client;

	head = (PROTO_HEAD *)get_head();

	int cmd = ENDION_FUNC_2(head->msg_id);

#ifdef FLOW_MONITOR
	add_one_other_server_request_msg(head);
#endif

	switch (cmd)
	{
		case SERVER_PROTO_DOUFACHANG_CHALLENGE_REQUEST:
		case SERVER_PROTO_DOUFACHANG_ADD_REWARD_REQUEST:
		case SERVER_PROTO_DOUFACHANG_BUY_CHALLENGE_REQUEST:
		case SERVER_PROTO_UNDO_COST:
		case SERVER_PROTO_DOUFACHANG_SYNC_RANK:
		case SERVER_PROTO_DOUFACHANG_SYNC_BUY_CHALLENGE:
			return transfer_to_gameserver();
	}

	extern_data = get_extern_data(head);
	LOG_DEBUG("[%s:%d]: Send Cmd To client, [%lu], cmd: %d", __FUNCTION__, __LINE__, extern_data->player_id, cmd);

	client = conn_node_client::get_nodes_by_player_id(extern_data->player_id);
	if (!client) {
		LOG_ERR("%s %d: can not find client[%u] [%lu], id = %d", __PRETTY_FUNCTION__, __LINE__, extern_data->fd, extern_data->player_id, cmd);
		return (0);
	}


	old_len = ENDION_FUNC_4(head->len);
	if (old_len < sizeof(PROTO_HEAD) + sizeof(EXTERN_DATA)) {
		LOG_ERR("%s %d: len wrong drop all msg", __FUNCTION__, __LINE__);
		return (-1);
	}
	head->len = ENDION_FUNC_4(old_len - sizeof(EXTERN_DATA));

	int ret = client->send_one_msg(head, 1);
	if (ret != 0) {
		LOG_ERR("%s %d: send to client[%lu] failed err[%d]", __FUNCTION__, __LINE__, extern_data->player_id, errno);
	}
	head->len = ENDION_FUNC_4(old_len);
	return (0);
}

conn_node_doufachang *conn_node_doufachang::server_node;
