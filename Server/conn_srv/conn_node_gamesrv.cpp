#include "conn_node_gamesrv.h"
#include "conn_node_raidsrv.h"
#include "conn_node_client.h"
#include "conn_node_login.h"
#include "conn_node_friend.h"
#include "conn_node_mail.h"
#include "conn_node_guild.h"
#include "conn_node_rank.h"
#include "conn_node_doufachang.h"
#include "conn_node_trade.h"
#include "conn_node_activity.h"
#include "game_event.h"
#include "flow_record.h"
#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "msgid.h"

#include "move.pb-c.h"
#include "login.pb-c.h"

conn_node_gamesrv::conn_node_gamesrv()
{
	send_buffer_begin_pos = 0;
	send_buffer_end_pos = 0;
	
	max_buf_len = 1024 * 1024;
	buf = (uint8_t *)malloc(max_buf_len + sizeof(EXTERN_DATA));
	assert(buf);	
}

conn_node_gamesrv::~conn_node_gamesrv()
{
	server_node = NULL;
}

int conn_node_gamesrv::recv_func(evutil_socket_t fd)
{
	for (;;) {
		int ret = get_one_buf();
		if (ret == 0) {
			if (dispatch_message() != 0) {
				LOG_INFO("%s %d: dispatch message failed", __FUNCTION__, __LINE__);
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

//分发消息到不同的服
int conn_node_gamesrv::dispatch_message()
{
	PROTO_HEAD *head = (PROTO_HEAD *)buf_head();
	uint32_t cmd = ENDION_FUNC_2(head->msg_id);
#ifdef FLOW_MONITOR
	add_one_other_server_request_msg(head);
#endif
	switch (cmd)
	{
		case SERVER_PROTO_BROADCAST:
			return broadcast_to_client();
		case SERVER_PROTO_BROADCAST_ALL:
			return broadcast_to_all_client();
		case SERVER_PROTO_KICK_ROLE_ANSWER:
			return kick_answer();
		case SERVER_PROTO_GAME_TO_FRIEND:
			return game_to_friendsrv();
		case SERVER_PROTO_GET_OFFLINE_CACHE_REQUEST:
		case SERVER_PROTO_CLEAR_OFFLINE_CACHE:
		case SERVER_PROTO_INSERT_OFFLINE_CACHE:
		case SERVER_PROTO_FRIEND_CHAT:
		case SERVER_PROTO_FRIEND_ADD_ENEMY:
		case SERVER_PROTO_FRIEND_RECOMMEND:
		case SERVER_PROTO_FRIEND_EXTEND_CONTACT_REQUEST:
		case SERVER_PROTO_FRIENDSRV_COST_ANSWER:
		case SERVER_PROTO_FRIEND_GIFT_COST_ANSWER:
		case SERVER_PROTO_FRIEND_TURN_SWITCH:
		case SERVER_PROTO_FRIEND_SYNC_RENAME:
		case SERVER_PROTO_FRIEND_TRACK_ENEMY_ANSWER:
		case SERVER_PROTO_FRIEND_DUMP_CLOSENESS_REQUEST:
			return transfer_to_friendsrv(); 
		case SERVER_PROTO_GAMESRV_START:
			transfer_to_guildsrv();
			return transfer_to_mailsrv();
		case SERVER_PROTO_MAIL_INSERT:
		case SERVER_PROTO_MAIL_GIVE_ATTACH_ANSWER:
			return transfer_to_mailsrv();
		case SERVER_PROTO_GUILDSRV_COST_ANSWER:
		case SERVER_PROTO_GUILDSRV_REWARD_ANSWER:
		case SERVER_PROTO_ADD_GUILD_RESOURCE:
		case SERVER_PROTO_SUB_GUILD_BUILDING_TIME:
		case SERVER_PROTO_GUILD_CHAT:
		case MSG_ID_GET_OTHER_INFO_ANSWER:
		case SERVER_PROTO_GM_DISBAND_GUILD:
		case SERVER_PROTO_GUILD_BATTLE_REWARD:
		case SERVER_PROTO_GUILD_BATTLE_ENTER_WAIT:
		case MSG_ID_GUILD_BATTLE_INFO_REQUEST:
		case SERVER_PROTO_GUILD_BATTLE_BEGIN:
		case SERVER_PROTO_GUILD_BATTLE_END:
		case SERVER_PROTO_GUILD_BATTLE_SETTLE:
		case SERVER_PROTO_GUILD_BATTLE_FINAL_LIST_REQUEST:
		case SERVER_PROTO_GUILD_PRODUCE_MEDICINE:
		case SERVER_PROTO_GUILD_ADD_FINAL_BATTLE_GUILD:
		case SERVER_PROTO_GUILD_RUQIN_CREAT_MONSTER_LEVEL_REQUEST:
		case SERVER_PROTO_GUILD_RUQIN_REWARD_INFO_NOTIFY:
		case SERVER_PROTO_GUILD_RUQIN_BOSS_CREAT_NOTIFY:
		case SERVER_PROTO_GUILD_ACCEPT_TASK_ANSWER:
		case SERVER_PROTO_GUILD_TASK_FINISH:
		case SERVER_PROTO_GUILD_BONFIRE_OPEN_ANSWER:
		case SERVER_PROTO_GUILD_BONFIRE_REWARD:
			return transfer_to_guildsrv();
		case SERVER_PROTO_REFRESH_PLAYER_REDIS_INFO:
		case SERVER_PROTO_WORLDBOSS_PLAYER_REDIS_INFO:
		case SERVER_PROTO_WORLDBOSS_BIRTH_UPDATA_REDIS_INFO:
			return transfer_to_ranksrv();
		case SERVER_PROTO_PLAYER_ONLINE_NOTIFY:
			transfer_to_ranksrv();
			transfer_to_friendsrv(); 
			transfer_to_doufachang();
			transfer_to_tradesrv();
			transfer_to_activitysrv();
			return transfer_to_guildsrv();
		case SERVER_PROTO_DOUFACHANG_ADD_REWARD_ANSWER:
		case SERVER_PROTO_DOUFACHANG_CHALLENGE_ANSWER:
		case SERVER_PROTO_DOUFACHANG_BUY_CHALLENGE_ANSWER:			
			return transfer_to_doufachang();
		case SERVER_PROTO_TRADE_ON_SHELF_REQUEST:
		case SERVER_PROTO_TRADE_ON_SHELF_DELETE_ITEM_ANSWER:
		case SERVER_PROTO_TRADE_OFF_SHELF_ADD_ITEM_ANSWER:
		case SERVER_PROTO_TRADE_RE_SHELF_CHANGE_ANSWER:
		case SERVER_PROTO_TRADESRV_COST_ANSWER:
		case SERVER_PROTO_TRADE_BUY_EXECUTE_ANSWER:
		case SERVER_PROTO_TRADE_SEND_RED_PACKET_REQUEST:
			return transfer_to_tradesrv();
		case SERVER_PROTO_TRADE_LOT_INSERT:
			transfer_to_guildsrv();
			return transfer_to_tradesrv();
		case SERVER_PROTO_RAID_PLAYER_ENTER_REQUEST:
		{
			EXTERN_DATA *extern_data = get_extern_data(head);
			conn_node_client *client = conn_node_client::get_nodes_by_player_id(extern_data->player_id);
			if (!client)
			{
				return (0);
			}
			client->raidsrv_id = 0;
		}
		case SERVER_PROTO_RAID_TEAM_ENTER_REQUEST:
		{
		}
		return transfer_to_raidsrv();
		case SERVER_PROTO_ACTIVITYSRV_REWARD_ANSWER:
			return transfer_to_activitysrv();
		default:
			return transfer_to_client();
	}
	return 0;
}

int conn_node_gamesrv::game_to_friendsrv()
{
	int ret = 0;
	PROTO_HEAD *head;
	head = (PROTO_HEAD *)buf_head();
	PROTO_HEAD *real_head = (PROTO_HEAD *)head->data;

	if (!conn_node_friend::server_node) {
		LOG_ERR("[%s:%d] do not have friend server connected", __FUNCTION__, __LINE__);
		ret = -1;
		goto done;
	}

	if (conn_node_friend::server_node->send_one_msg(real_head, 1) != (int)ENDION_FUNC_4(real_head->len)) {
		LOG_ERR("[%s:%d] send to friend failed err[%d]", __FUNCTION__, __LINE__, errno);
		ret = -2;
		goto done;
	}
#ifdef FLOW_MONITOR
	add_on_other_server_answer_msg(head);
#endif
done:	
	return (ret);
}

int conn_node_gamesrv::transfer_to_friendsrv()
{
	int ret = 0;
	PROTO_HEAD *head;
	head = (PROTO_HEAD *)buf_head();
//	PROTO_HEAD *real_head = (PROTO_HEAD *)head->data;

	if (!conn_node_friend::server_node) {
		LOG_ERR("[%s:%d] do not have friend server connected", __FUNCTION__, __LINE__);
		ret = -1;
		goto done;
	}

	if (conn_node_friend::server_node->send_one_msg(head, 1) != (int)ENDION_FUNC_4(head->len)) {
		LOG_ERR("[%s:%d] send to friend failed err[%d]", __FUNCTION__, __LINE__, errno);
		ret = -2;
		goto done;
	}
#ifdef FLOW_MONITOR
	add_on_other_server_answer_msg(head);
#endif
done:	
	return (ret);
}

int conn_node_gamesrv::transfer_to_mailsrv()
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
int conn_node_gamesrv::transfer_to_raidsrv()
{
	int ret = 0;
	PROTO_HEAD *head;
	head = (PROTO_HEAD *)buf_head();

	if (!conn_node_raidsrv::server_node[0]) {
		LOG_ERR("[%s:%d] do not have raidsrv connected", __FUNCTION__, __LINE__);
		ret = -1;
		goto done;
	}

	if (conn_node_raidsrv::server_node[0]->send_one_msg(head, 1) != (int)ENDION_FUNC_4(head->len)) {
		LOG_ERR("[%s:%d] send to raid failed err[%d]", __FUNCTION__, __LINE__, errno);
		ret = -2;
		goto done;
	}
#ifdef FLOW_MONITOR
	add_on_other_server_answer_msg(head);
#endif
done:	
	return (ret);
}
int conn_node_gamesrv::transfer_to_doufachang()
{
	int ret = 0;
	PROTO_HEAD *head;
	head = (PROTO_HEAD *)buf_head();

	if (!conn_node_doufachang::server_node) {
		LOG_ERR("[%s:%d] do not have doufachang server connected", __FUNCTION__, __LINE__);
		ret = -1;
		goto done;
	}

	if (conn_node_doufachang::server_node->send_one_msg(head, 1) != (int)ENDION_FUNC_4(head->len)) {
		LOG_ERR("[%s:%d] send to doufachang failed err[%d]", __FUNCTION__, __LINE__, errno);
		ret = -2;
		goto done;
	}
#ifdef FLOW_MONITOR
	add_on_other_server_answer_msg(head);
#endif
done:
	return (ret);
}

int conn_node_gamesrv::transfer_to_guildsrv()
{
	int ret = 0;
	PROTO_HEAD *head;
	head = (PROTO_HEAD *)buf_head();

	if (!conn_node_guild::server_node) {
		LOG_ERR("[%s:%d] do not have guild server connected", __FUNCTION__, __LINE__);
		ret = -1;
		goto done;
	}

	if (conn_node_guild::server_node->send_one_msg(head, 1) != (int)ENDION_FUNC_4(head->len)) {
		LOG_ERR("[%s:%d] send to guild failed err[%d]", __FUNCTION__, __LINE__, errno);
		ret = -2;
		goto done;
	}
#ifdef FLOW_MONITOR
	add_on_other_server_answer_msg(head);
#endif
done:	
	return (ret);
}

int conn_node_gamesrv::transfer_to_ranksrv()
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

int conn_node_gamesrv::transfer_to_tradesrv()
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

int conn_node_gamesrv::transfer_to_activitysrv()
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
#ifdef FLOW_MONITOR
	add_on_other_server_answer_msg(head);
#endif
done:	
	return (ret);
}

int conn_node_gamesrv::transfer_to_client()
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
//	if (ret != ENDION_FUNC_4(head->len)) {
		LOG_ERR("%s %d: send to client[%lu] failed err[%d]", __FUNCTION__, __LINE__, extern_data->player_id, errno);
//		head->len = ENDION_FUNC_4(old_len);		
//		return (0);
	}
	head->len = ENDION_FUNC_4(old_len);
	return (0);
}

int conn_node_gamesrv::broadcast_to_all_client()
{
	conn_node_client *client;	
	PROTO_HEAD *real_head;
	PROTO_HEAD_CONN_BROADCAST *head = (PROTO_HEAD_CONN_BROADCAST *)buf_head();
	real_head = &head->proto_head;

	for (std::map<evutil_socket_t, conn_node_client *>::iterator it = conn_node_client::map_fd_nodes.begin();
		 it != conn_node_client::map_fd_nodes.end(); ++it)
	{
		client = it->second;
		if (!client || client->player_id == 0) {
			continue;
		}
//		LOG_DEBUG("%s %d: broadcast to playerid[%lu] openid[%u] fd[%u]", __FUNCTION__, __LINE__, client->player_id, client->open_id, client->fd);		
		
		if (client->send_one_msg(real_head, 1) != (int)(ENDION_FUNC_4(real_head->len))) {		
//			LOG_ERR("%s: broadcast to client[%u] failed err[%d]", __FUNCTION__, client->player_id, errno);
			continue;
		}
	}
	return (0);
}

int conn_node_gamesrv::broadcast_to_client()
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

int conn_node_gamesrv::kick_role(EXTERN_DATA *extern_data)
{
	conn_node_client *client;	
 	client = conn_node_client::get_nodes_by_player_id(extern_data->player_id);
	if (client)
		remove_listen_callback_event(client);	
	return (0);
}

int conn_node_gamesrv::kick_answer()
{
	int ret = 0;
	PROTO_HEAD *head;
	head = (PROTO_HEAD *)buf_head();
	EXTERN_DATA* extern_data = get_extern_data(head);

	CommAnswer *ans = comm_answer__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!ans)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return (-10);
	}

	if (ans->result != 0)
	{
		if (extern_data->open_id == 0)
		{
			return 0;
		}

		conn_node_client *client = conn_node_client::get_nodes_by_open_id(extern_data->open_id);	
		if (!client) {
			LOG_ERR("%s %d: can not find client, fd[%u] open_id[%u]", __PRETTY_FUNCTION__, __LINE__, extern_data->fd, extern_data->open_id);		
			return (0);		
		}

		PlayerListAnswer resp;
		player_list_answer__init(&resp);
		resp.result = ans->result;

		PROTO_HEAD *head_send = get_send_buf(MSG_ID_PLAYER_LIST_ANSWER, head->seq);
		size_t pack_size = player_list_answer__pack(&resp, (uint8_t *)&head_send->data[0]);
		if (pack_size != (size_t)-1)
		{
			head_send->len = ENDION_FUNC_4(sizeof(PROTO_HEAD) + pack_size);
			int ret = client->send_one_msg(head_send, 1);
			if (ret != 0) {
				LOG_ERR("%s %d: send to client[%lu] failed err[%d]", __FUNCTION__, __LINE__, extern_data->player_id, errno);
			}
		}

		return (0);
	}

	head = get_send_buf(MSG_ID_PLAYER_LIST_REQUEST, 0);
	head->len = htons(sizeof(PROTO_HEAD));

	add_extern_data(head, extern_data);

	conn_node_login *server = conn_node_login::get_normal_node();
	if (!server) {
		LOG_ERR("[%s: %d]: do not have login server connected", __FUNCTION__, __LINE__);
		ret = -1;
		goto done;
	}

	if (server->send_one_msg(head, 1) != (int)ENDION_FUNC_4(head->len)) {
		LOG_ERR("[%s: %d]: send to login server failed err[%d]", __FUNCTION__, __LINE__, errno);
		ret = -2;
		goto done;
	}
	
#ifdef FLOW_MONITOR
	add_on_other_server_answer_msg(head);
#endif

done:
	return (ret);
}

int conn_node_gamesrv::add_cached_buf(PROTO_HEAD *head)
{
	int len = ENDION_FUNC_4(head->len);

	LOG_DEBUG("%s %d: add cached buf[%d]", __FUNCTION__, __LINE__, len);
	if (cached_len + len >= (int)sizeof(cached_buf)) {
		LOG_ERR("%s %d: len[%d] >= cached_len[%d]", __FUNCTION__, __LINE__, len, cached_len);
		return (-1);
	}
	memcpy(&cached_buf[cached_len], head, len);
	cached_len += len;
	return (0);
}

int conn_node_gamesrv::send_all_cached_buf()
{
	if (!server_node)
		return (-1);
	int pos = 0;
	int len;
	PROTO_HEAD *head;
	for (; pos < cached_len;) {
		head = (PROTO_HEAD *)&cached_buf[pos];
		len = ENDION_FUNC_4(head->len);
		if (pos + len > cached_len)
			goto fail;
		if (server_node->send_one_msg(head, 0) != len) {
			goto fail;
		}
		pos += len;
	}
//		if (conn_node_gamesrv::server_node->send_one_msg(head, 1) != ENDION_FUNC_4(head->len)) {		
	return (0);

fail:
	if (pos != 0) {
		assert(cached_len <= pos);
		memmove(&cached_buf[0], &cached_buf[pos], cached_len - pos);
	}
	return (-1);
}


int conn_node_gamesrv::send_one_msg(PROTO_HEAD *head, uint8_t force) {
#if 1
//	static int seq = 1;
	char *p = (char *)head;
	int len = ENDION_FUNC_4(head->len);
//	head->seq = ENDION_FUNC_2(seq++);

	if (send_buffer_end_pos+len >= MAX_GAMESRV_SEND_BUFFER_SIZE) {  ///缓冲区溢出, 关闭连接
		LOG_ERR("[%s: %d]: fd: %d: send msg[%d] len[%d], seq[%d] buffer full, begin[%d] end[%d]", __PRETTY_FUNCTION__, __LINE__, fd, ENDION_FUNC_2(head->msg_id), ENDION_FUNC_4(head->len), ENDION_FUNC_2(head->seq), send_buffer_begin_pos, send_buffer_end_pos);
		return -1;
	}


	if (head->msg_id != MSG_ID_HEARTBEAT_NOTIFY)
		LOG_DEBUG("[%s: %d]: fd: %d: send msg[%d] len[%d], seq[%d] , begin[%d] end[%d]", __PRETTY_FUNCTION__, __LINE__, fd, ENDION_FUNC_2(head->msg_id), ENDION_FUNC_4(head->len), ENDION_FUNC_2(head->seq), send_buffer_begin_pos, send_buffer_end_pos);

	memcpy(send_buffer+send_buffer_end_pos, p, len);
//	encoder_data((PROTO_HEAD*)(send_buffer+send_buffer_end_pos));

	send_buffer_end_pos += len;

	if (send_buffer_begin_pos == 0) {
		int result = event_add(&this->ev_write, NULL);
		if (0 != result) {
			LOG_ERR("[%s : %d]: event add failed, result: %d", __PRETTY_FUNCTION__, __LINE__, result);
			return result;
		}
	}

	return len;
#else
	return conn_node_base::send_one_msg(head, force);
#endif
}

void on_gamesrv_write(int fd, short ev, void *arg) {
	assert(arg);
	conn_node_gamesrv *client = (conn_node_gamesrv *)arg;
	client->send_data_to_server();
}

void conn_node_gamesrv::send_data_to_server() {
	if (send_buffer_end_pos-send_buffer_begin_pos<=0)
		return;

	int len = write(this->fd, send_buffer + send_buffer_begin_pos, send_buffer_end_pos-send_buffer_begin_pos);

	LOG_DEBUG("%s %d: write to fd: %u: ret %d, end pos = %d, begin pos = %d", __PRETTY_FUNCTION__, __LINE__, fd, len, send_buffer_end_pos, send_buffer_begin_pos);

	if (len == -1) {  //发送失败
		if (errno == EINTR || errno == EAGAIN) {
			int result = event_add(&this->ev_write, NULL);
			if (0 != result) {
				LOG_ERR("[%s : %d]: event add failed, result: %d", __PRETTY_FUNCTION__, __LINE__, result);
				return;
			}
		}
	}
	else if (send_buffer_begin_pos + len < send_buffer_end_pos) {  //没发完
		send_buffer_begin_pos += len;
		int result = event_add(&this->ev_write, NULL);
		if (0 != result) {
			LOG_ERR("[%s : %d]: event add failed, result: %d", __PRETTY_FUNCTION__, __LINE__, result);
			return;
		}
	}
	else {  //发完了
		send_buffer_begin_pos = send_buffer_end_pos = 0;
		return;
	}


	/// 当数据发送完毕后pos归0
//	if (send_buffer_begin_pos == send_buffer_end_pos) {
//		send_buffer_begin_pos = send_buffer_end_pos = 0;
//		return;
//	}

	if (send_buffer_end_pos>=MAX_GAMESRV_SEND_BUFFER_SIZE/2 && (send_buffer_begin_pos/1024) > 0 && send_buffer_end_pos>send_buffer_begin_pos) {
		int sz = send_buffer_end_pos - send_buffer_begin_pos;
		memmove(send_buffer, send_buffer+send_buffer_begin_pos, sz);
		send_buffer_begin_pos = 0;
		send_buffer_end_pos = sz;
	}
}


//////////////////////////// 下面是static 函数
conn_node_gamesrv *conn_node_gamesrv::server_node;
char conn_node_gamesrv::cached_buf[1024 * 1024 * 1024];
int conn_node_gamesrv::cached_len;
