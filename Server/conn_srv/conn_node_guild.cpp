#include "conn_node_guild.h"
#include "conn_node_client.h"
#include "conn_node_gamesrv.h"
#include "conn_node_mail.h"
#include "conn_node_rank.h"
#include "conn_node_activity.h"
#include "conn_node_trade.h"
#include "game_event.h"
#include "flow_record.h"
#include <assert.h>
#include <errno.h>
#include "msgid.h"
#include <stdlib.h>

conn_node_guild *conn_node_guild::server_node;

conn_node_guild::conn_node_guild()
{
	max_buf_len = 1024 * 1024;
	buf = (uint8_t *)malloc(max_buf_len + sizeof(EXTERN_DATA));
	assert(buf);	
}

conn_node_guild::~conn_node_guild()
{
	server_node = NULL;
}

int conn_node_guild::recv_func(evutil_socket_t fd)
{
	for (;;) {
		int ret = get_one_buf();
		if (ret == 0) {
			if (dispatch_message() != 0) {
				LOG_INFO("%s %d: dispatch_message failed", __FUNCTION__, __LINE__);
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

int conn_node_guild::dispatch_message()
{
	PROTO_HEAD *head = (PROTO_HEAD *)buf_head();
	uint32_t cmd = ENDION_FUNC_2(head->msg_id);
#ifdef FLOW_MONITOR
	add_one_other_server_request_msg(head);
#endif

	//转发到其他服务器的消息
	switch (cmd)
	{
		case SERVER_PROTO_BROADCAST:
			return broadcast_to_client();
		case SERVER_PROTO_GUILDSRV_COST_REQUEST:
		case SERVER_PROTO_GUILDSRV_REWARD_REQUEST:
		case SERVER_PROTO_SYNC_GUILD_SKILL:
		case SERVER_PROTO_SYNC_GUILD_INFO:
		case SERVER_PROTO_GUILD_DISBAND:
		case SERVER_PROTO_GUILD_ANSWER_AWARD:
		case MSG_ID_GUILD_BATTLE_CALL_REQUEST:
		case SERVER_PROTO_GUILD_BATTLE_FINAL_LIST_ANSWER:
		case SERVER_PROTO_UNDO_COST:
		case SERVER_PROTO_GUILD_PRODUCE_MEDICINE:
		case SERVER_PROTO_GUILD_SYNC_DONATION:
		case SERVER_PROTO_GUILD_SKILL_LEVEL_UP:
		case SERVER_PROTO_GUILD_SYNC_ALL:
		case SERVER_PROTO_GUILD_CREATE:
		case SERVER_PROTO_GUILD_RENAME:
		case SERVER_PROTO_GUILD_RUQIN_CREAT_MONSTER_LEVEL_ANSWER:
		case SERVER_PROTO_GUILD_ACCEPT_TASK_REQUEST:
		case SERVER_PROTO_GUILD_SYNC_TASK:
		case SERVER_PROTO_GUILD_RUQIN_ADD_COUNT:
		case SERVER_PROTO_GUILD_RUQIN_SYNC_COUNT:
		case SERVER_PROTO_GUILD_SYNC_DONATE:
		case SERVER_PROTO_GUILD_SYNC_PARTICIPATE_ANSWER:
		case SERVER_PROTO_GUILD_BONFIRE_OPEN_REQUEST:
			return transfer_to_gamesrv();
		case SERVER_PROTO_MAIL_INSERT:
			return transfer_to_mailsrv();
		case SERVER_PROTO_REFRESH_PLAYER_REDIS_INFO:
		case SERVER_PROTO_REFRESH_GUILD_REDIS_INFO:
			return transfer_to_ranksrv();
		case SERVER_PROTO_ACTIVITY_SHIDAMENZONG_GIVE_REWARD_ANSWER:
			return transfer_to_activitysrv();
		case SERVER_PROTO_TRADE_SEND_RED_PACKET_REQUEST:
			return transfer_to_tradesrv();
		default:
			return transfer_to_client();
	}
	return 0;
}

int conn_node_guild::transfer_to_client()
{
	uint32_t old_len;
	PROTO_HEAD *head;
	EXTERN_DATA *extern_data;
	conn_node_client *client;

	head = (PROTO_HEAD *)get_head();

	int cmd = ENDION_FUNC_2(head->msg_id);

	
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

int conn_node_guild::transfer_to_gamesrv()
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

int conn_node_guild::transfer_to_mailsrv()
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

int conn_node_guild::transfer_to_ranksrv()
{
	int ret = 0;
	PROTO_HEAD *head;
	head = (PROTO_HEAD *)buf_head();

	if (!conn_node_rank::server_node) {
		LOG_ERR("[%s:%d] do not have rank server connected", __FUNCTION__, __LINE__);
		ret = -1;
		goto done;
	}

	if (conn_node_rank::server_node->send_one_msg(head, 1) != (int)ENDION_FUNC_4(head->len)) {
		LOG_ERR("[%s:%d] send to rank failed err[%d]", __FUNCTION__, __LINE__, errno);
		ret = -2;
		goto done;
	}
#ifdef FLOW_MONITOR
	add_on_other_server_answer_msg(head);
#endif
done:	
	return (ret);
}

int conn_node_guild::transfer_to_activitysrv()
{
	int ret = 0;
	PROTO_HEAD *head;
	head = (PROTO_HEAD *)buf_head();

	if (!conn_node_activity::server_node) {
		LOG_ERR("[%s:%d] do not have activity server connected", __FUNCTION__, __LINE__);
		ret = -1;
		goto done;
	}

	if (conn_node_activity::server_node->send_one_msg(head, 1) != (int)ENDION_FUNC_4(head->len)) {
		LOG_ERR("[%s:%d] send to activity failed err[%d]", __FUNCTION__, __LINE__, errno);
		ret = -2;
		goto done;
	}
done:	
	return (ret);
}

int conn_node_guild::broadcast_to_client()
{
	uint64_t player_id_offset;
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
			LOG_ERR("%s %d: can not find client[%lu] msg[%u]", __FUNCTION__, __LINE__, player_id[i], ENDION_FUNC_2(real_head->msg_id));
			continue;
		}
		
//		LOG_DEBUG("%s %d: broadcast to playerid[%lu] openid[%u] fd[%u] msg[%u]", __FUNCTION__, __LINE__, player_id[i], client->open_id, client->fd, ENDION_FUNC_2(real_head->msg_id));		
		
		if (client->send_one_msg(real_head, 1) != (int)(ENDION_FUNC_4(real_head->len))) {		
//			LOG_ERR("%s %d: broadcast to client[%u] player[%lu] failed err[%d]", __FUNCTION__, __LINE__, client->fd, player_id[i], errno);
			continue;
		}
	}
	return (ret);
}
int conn_node_guild::transfer_to_tradesrv()
{
	int ret = 0;
	PROTO_HEAD *head;
	head = (PROTO_HEAD *)buf_head();

	if (!conn_node_trade::server_node) {
		LOG_ERR("[%s:%d] do not have trade server connected", __FUNCTION__, __LINE__);
		ret = -1;
		goto done;
	}

	if (conn_node_trade::server_node->send_one_msg(head, 1) != (int)ENDION_FUNC_4(head->len)) {
		LOG_ERR("[%s:%d] send to trade failed err[%d]", __FUNCTION__, __LINE__, errno);
		ret = -2;
		goto done;
	}
#ifdef FLOW_MONITOR
	add_on_other_server_answer_msg(head);
#endif
done:	
	return (ret);
}

