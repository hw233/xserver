#include "conn_node_dump.h"
#include "conn_node_client.h"
#include "conn_node_login.h"
#include "conn_node_mail.h"
#include "game_event.h"
#include "flow_record.h"
#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include "msgid.h"

#include "move.pb-c.h"

conn_node_dump::conn_node_dump()
{
	player_id = 0;
	max_buf_len = 10 * 1024;
	buf = (uint8_t *)malloc(max_buf_len + sizeof(EXTERN_DATA));
	assert(buf);	
}

conn_node_dump::~conn_node_dump()
{
	server_node = NULL;
}

int conn_node_dump::recv_func(evutil_socket_t fd)
{
	for (;;) {
		int ret = get_one_buf();
		if (ret == 0) {
			if (transfer_to_client() != 0) {
				LOG_INFO("%s %d: transfer to client failed", __FUNCTION__, __LINE__);
//				ret = remove_one_buf();
//				return (0);
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

/** dump_srv涓巆onn_srv閾炬帴澶勭悊 **/
int conn_node_dump::transfer_to_client()
{
//	uint16_t old_len;
	PROTO_HEAD *head;
	__attribute__((unused)) EXTERN_DATA *extern_data;
//	conn_node_client *client;

	head = (PROTO_HEAD *)get_head();

	int cmd = ENDION_FUNC_2(head->msg_id);

#ifdef FLOW_MONITOR
	add_one_other_server_request_msg(head);
#endif

	
	switch (cmd)
	{
		case SERVER_PROTO_MAIL_INSERT:
			transfer_to_mailsrv();
			break;
	}
	
	extern_data = get_extern_data(head);
	LOG_DEBUG("[%s:%d]: Send Cmd To client, [%lu], cmd: %d", __FUNCTION__, __LINE__, extern_data->player_id, cmd);

/* 	client = conn_node_client::get_nodes_by_player_id(extern_data->player_id);	
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
//	if (ret != ENDION_FUNC_4(head->len)) {
		LOG_ERR("%s %d: send to client failed err[%d]", __FUNCTION__, __LINE__, errno);
//		head->len = ENDION_FUNC_4(old_len);		
//		return (0);
	}
	head->len = ENDION_FUNC_4(old_len);*/
	return (0);
}

int conn_node_dump::transfer_to_mailsrv()
{
	int ret = 0;
	PROTO_HEAD *head;
	head = (PROTO_HEAD *)buf_head();

	if (!conn_node_mail::server_node) {
		LOG_ERR("[%s:%d] do not have mail server connected", __FUNCTION__, __LINE__);
		ret = -1;
		goto done;
	}

	if (conn_node_mail::server_node->send_one_msg(head, 1) != (int)ENDION_FUNC_4(head->len)) {
		LOG_ERR("[%s:%d] send to mail failed err[%d]", __FUNCTION__, __LINE__, errno);
		ret = -2;
		goto done;
	}

#ifdef FLOW_MONITOR
	add_on_other_server_answer_msg(head);
#endif


done:	
	return (ret);
}

int conn_node_dump::broadcast_to_all_client()
{
/*	conn_node_client *client;	
	PROTO_HEAD *real_head;
	PROTO_HEAD_CONN_BROADCAST *head = (PROTO_HEAD_CONN_BROADCAST *)buf_head();
	real_head = &head->proto_head;

	for (std::map<evutil_socket_t, conn_node_client *>::iterator it = conn_node_client::map_fd_nodes.begin();
		 it != conn_node_client::map_fd_nodes.end(); ++it)
	{
		client = it->second;
		if (!client) {
			continue;
		}
//		LOG_DEBUG("%s %d: broadcast to playerid[%lu] openid[%u] fd[%u]", __FUNCTION__, __LINE__, client->player_id, client->open_id, client->fd);		
		
		if (client->send_one_msg(real_head, 1) != (int)(ENDION_FUNC_4(real_head->len))) {		
//			LOG_ERR("%s: broadcast to client[%u] failed err[%d]", __FUNCTION__, client->player_id, errno);
			continue;
		}
	}*/
	return (0);
}

int conn_node_dump::broadcast_to_client()
{
/*	uint64_t player_id_offset;
	uint64_t *player_id;
	conn_node_client *client;	
	int i;
	int ret = 0;
	PROTO_HEAD *real_head;
	PROTO_HEAD_CONN_BROADCAST *head = (PROTO_HEAD_CONN_BROADCAST *)buf_head();
	real_head = &head->proto_head;
	player_id_offset = ENDION_FUNC_4(real_head->len) + offsetof(PROTO_HEAD_CONN_BROADCAST, proto_head);
	player_id = (uint64_t *)((char *)head + player_id_offset);

	//check len
	if (player_id_offset + sizeof(uint64_t) * head->num_player_id != ENDION_FUNC_4(head->len)) {
		LOG_ERR("%s: broadcast len wrong [%u] [%u]", __FUNCTION__, ENDION_FUNC_4(real_head->len), ENDION_FUNC_4(head->len));
		return (-1);		
	}
	
	for (i = 0; i < head->num_player_id; ++i) {
		client = conn_node_client::get_nodes_by_player_id(player_id[i]);	
		if (!client) {
			LOG_ERR("%s %d: can not find client[%lu]", __FUNCTION__, __LINE__, player_id[i]);
			continue;
		}

		if (real_head->msg_id == 10102)
		{
			int old_len = ENDION_FUNC_4(real_head->len);
			int len = ENDION_FUNC_4(old_len - sizeof(PROTO_HEAD));
			MoveNotify *notify = move_notify__unpack(NULL, len, (uint8_t *)(&real_head->data[0]));
			if (notify)
			{
				LOG_INFO("player[%lu] move begin", notify->playerid);
				for (uint32_t i = 0; i < notify->n_data; ++i)
				{
					LOG_INFO("player[%lu] move [%.1f][%.1f]", notify->data[i]->pos_x, notify->data[i]->pos_z);
				}
				LOG_INFO("player[%lu] move end", notify->playerid);
				move_notify__free_unpacked(notify, NULL);				
			}
		}

		if (real_head->msg_id == 10103)
		{
			int old_len = ENDION_FUNC_4(real_head->len);
			int len = ENDION_FUNC_4(old_len - sizeof(PROTO_HEAD));
			SightChangedNotify *notify = sight_changed_notify__unpack(NULL, len, (uint8_t *)(&real_head->data[0]));
			if (notify)
			{
				LOG_INFO("send sightchangednotify, data_len = %d addplayer num = %d", len, notify->n_add_player);
				for (uint32_t i = 0; i < notify->n_add_player; ++i)
				{
					LOG_INFO("send sightchangednotify, addplayer name = %s, move_len[%d]",
						notify->add_player[i]->name, notify->add_player[i]->n_data);
					if (notify->add_player[i]->n_data > 0)
						LOG_INFO("send sightchangednotify, [%f][%f]",
							notify->add_player[i]->data[0]->pos_x,
							notify->add_player[i]->data[0]->pos_z);
				}
				sight_changed_notify__free_unpacked(notify, NULL);
			}
			else
			{
				LOG_ERR("send sightchangednotify fail");				
			}
		}

		
//		LOG_DEBUG("%s %d: broadcast to playerid[%lu] openid[%u] fd[%u] msg[%u]", __FUNCTION__, __LINE__, player_id[i], client->open_id, client->fd, ENDION_FUNC_2(real_head->msg_id));		
		
		if (client->send_one_msg(real_head, 1) != (int)(ENDION_FUNC_4(real_head->len))) {		
//			LOG_ERR("%s %d: broadcast to client[%u] player[%lu] failed err[%d]", __FUNCTION__, __LINE__, client->fd, player_id[i], errno);
			continue;
		}
	}
	return (ret);*/
	return 0;
}

int conn_node_dump::kick_role(EXTERN_DATA *extern_data)
{
	conn_node_client *client;	
 	client = conn_node_client::get_nodes_by_player_id(extern_data->player_id);
	if (client)
		remove_listen_callback_event(client);	
	return (0);
}

int conn_node_dump::kick_answer()
{
	int ret = 0;
	PROTO_HEAD *head;
	head = (PROTO_HEAD *)buf_head();
	EXTERN_DATA* ext_data = get_extern_data(head);

	head = get_send_buf(MSG_ID_PLAYER_LIST_REQUEST, 0);
	head->len = htons(sizeof(PROTO_HEAD));

	add_extern_data(head, ext_data);

	conn_node_login *server = conn_node_login::get_normal_node();
	if (!server) {
		LOG_ERR("[%s: %d]: do not have friend server connected", __FUNCTION__, __LINE__);
		ret = -1;
		goto done;
	}

	if (server->send_one_msg(head, 1) != htons(head->len)) {
		LOG_ERR("[%s: %d]: send to login server failed err[%d]", __FUNCTION__, __LINE__, errno);
		ret = -2;
		goto done;
	}
done:
	return (ret);
}


//////////////////////////// 涓嬮潰鏄痵tatic 鍑芥暟
conn_node_dump *conn_node_dump::server_node;
uint64_t conn_node_dump::player_id = 0;
