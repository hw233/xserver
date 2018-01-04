#include "conn_node_client.h"
#include "conn_node_gamesrv.h"
#include "conn_node_raidsrv.h"
#include "conn_node_login.h"
#include "conn_node_dump.h"
#include "conn_node_mail.h"
#include "conn_node_friend.h"
#include "conn_node_doufachang.h"
#include "conn_node_guild.h"
#include "conn_node_rank.h"
#include "conn_node_trade.h"
#include "conn_node_activity.h"
#include "time_helper.h"
#include "game_event.h"
#include "tea.h"
#include "msgid.h"
#include "flow_record.h"
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>

#define UNUSED(x) (void)(x)

conn_node_client::conn_node_client()
{
	open_id = 0;
	player_id = 0;
	raidsrv_id = -1;

	send_buffer_begin_pos = 0;
	send_buffer_end_pos = 0;

	max_buf_len = 10 * 1024;
	buf = (uint8_t *)malloc(max_buf_len + sizeof(EXTERN_DATA));
	assert(buf);
}

conn_node_client::~conn_node_client()
{
	LOG_DEBUG("%s %d: openid[%u] fd[%u] playerid[%lu]", __PRETTY_FUNCTION__, __LINE__, open_id, fd, player_id);
	map_fd_nodes.erase(fd);
	if (open_id > 0) {
		map_open_id_nodes.erase(open_id);
	}
	if (player_id > 0) {
		map_player_id_nodes.erase(player_id);
	} else {
		return;
	}

	send_player_exit();
}

int conn_node_client::send_player_exit(bool again/* = false*/)
{
	if (player_id == 0)
	{
		return 0;
	}

	//通知各个服，玩家下线
	PROTO_ROLE_KICK data;
	data.head.len = ENDION_FUNC_4(sizeof(PROTO_ROLE_KICK));
	data.head.msg_id = ENDION_FUNC_2(SERVER_PROTO_KICK_ROLE_NOTIFY);
	data.again = (again ? 1 : 0); //玩家是否重新选角，如果是，需要回复确认
	data.extern_data.player_id = player_id;
	data.extern_data.open_id = open_id;
	data.extern_data.fd = fd;
	data.extern_data.port = sock.sin_port;

	if (!conn_node_gamesrv::server_node) {
		if (conn_node_gamesrv::add_cached_buf(&data.head) != 0) {
			LOG_ERR("%s %d: add cached buf failed", __PRETTY_FUNCTION__, __LINE__);
		}
	}
	else if (conn_node_gamesrv::server_node->send_one_msg(&data.head, 1) != (int)(ENDION_FUNC_4(data.head.len))) {
		LOG_ERR("%s: send to gameserver failed err[%d]", __PRETTY_FUNCTION__, errno);
	}
	//通知好友服
	if (!conn_node_friend::server_node || conn_node_friend::server_node->send_one_msg(&data.head, 1) != (int)ENDION_FUNC_4(data.head.len))
	{
		LOG_ERR("%s: send to friend_srv failed err[%d]", __PRETTY_FUNCTION__, errno);
	}

	//清除角色信息，放最后
	map_player_id_nodes.erase(player_id);
	player_id = 0;

	return 0;
}

int conn_node_client::decode_and_check_crc(PROTO_HEAD *head)
{
/*
	int len = ENDION_FUNC_4(head->len) - sizeof(uint16_t);
	char *p = (char *)&head->msg_id;
	int pos = 0;
	while (pos + 8 <= len)
	{
		sg_decrypt((uint32_t *)&p[pos]);
		pos += 8;
	}
		//check seq

		//心跳包不检测
	if (ENDION_FUNC_2(head->msg_id) == 0)
	{
		++seq;
		return (0);
	}

	if (ENDION_FUNC_2(head->msg_id) == LOGIN_REQUEST)
		seq = ENDION_FUNC_2(head->seq);

	if (seq != ENDION_FUNC_2(head->seq))
	{
		LOG_ERR("%s %d: check seq fail, msg id = %d seq[%d] head->seq[%d]", __PRETTY_FUNCTION__, __LINE__, ENDION_FUNC_2(head->msg_id), seq, ENDION_FUNC_2(head->seq));
		return (-1);
	}

	uint32_t crc32 = crc32_long((u_char*)head->data, ENDION_FUNC_4(head->len) - sizeof(PROTO_HEAD));
	if (head->crc != ENDION_FUNC_4(crc32))
	{
		LOG_ERR("%s %d: check crc fail, msg id = %d seq[%d] head->seq[%d]", __PRETTY_FUNCTION__, __LINE__, ENDION_FUNC_2(head->msg_id), seq, ENDION_FUNC_2(head->seq));
		return (-10);
	}
	++seq;
*/
	return (0);
}

int conn_node_client::send_hello_resp()
{
	struct timeval tv;
	time_helper::get_current_time(&tv);
//	tv.tv_sec;
	return (0);
}

int conn_node_client::recv_func(evutil_socket_t fd)
{
	PROTO_HEAD *head;
	uint32_t old_len;
	EXTERN_DATA save_data;
	EXTERN_DATA *extern_data;

	for (;;) {
		int ret = get_one_buf();
		if (ret == 0) {
			head = (PROTO_HEAD *)buf_head();

			if (decode_and_check_crc(head) != 0) {
				LOG_INFO("%s %d: crc err, connect closed from fd %u, err = %d", __PRETTY_FUNCTION__, __LINE__, fd, errno);
				return (-1);
			}

			uint32_t cmd = ENDION_FUNC_2(head->msg_id);
			if (0 == cmd) {
				send_hello_resp();
//				this->send_one_msg(head, 1);
			} else {
				old_len = ENDION_FUNC_4(head->len);
				extern_data = (EXTERN_DATA *)&head->data[old_len - sizeof(PROTO_HEAD)];
				memcpy(&save_data, extern_data, sizeof(EXTERN_DATA));
				head->len = ENDION_FUNC_4(old_len + sizeof(EXTERN_DATA));

				extern_data->player_id = player_id;
				extern_data->open_id = open_id;
				extern_data->fd = fd;
				extern_data->port = sock.sin_port;

				transfer_to_dumpserver(head);
#ifdef FLOW_MONITOR
				add_one_client_request(head);
#endif
				if (player_id == 0) {
					if (transfer_to_loginserver() != 0) {
						remove_listen_callback_event(this);
						return (0);
					}
				} else if (dispatch_message() != 0) {
				}

//				transfer_to_gameserver();
				head->len = ENDION_FUNC_4(old_len);
				memcpy(extern_data, &save_data, sizeof(EXTERN_DATA));
			}
		}

		if (ret < 0) {
			LOG_INFO("%s: connect closed from fd %u, err = %d", __PRETTY_FUNCTION__, fd, errno);
//		send_logout_request();
//		del_client_map_by_fd(fd, &client_maps[0], (int *)&num_client_map);
			return (-1);
		} else if (ret > 0) {
			break;
		}

		ret = remove_one_buf();
	}
	return (0);
}

void encoder_data(PROTO_HEAD *head) {
/*
	size_t sz = ENDION_FUNC_4(head->len) - sizeof(uint16_t);
	if (sz <= 0)
		return;

	char* pData = (char*)head+sizeof(uint16_t);
	uint32_t crc32 = crc32_long((u_char*)head->data, ENDION_FUNC_4(head->len) - sizeof(PROTO_HEAD));
	head->crc = ENDION_FUNC_4(crc32);

	char * p = (char *)(pData);

	for (size_t i=0; i<sz/8; ++i) {
		sg_encrypt((uint32_t *)p);
		p += 8;
	}
*/
}

int conn_node_client::send_one_msg(PROTO_HEAD *head, uint8_t force) {
	{
		static uint8_t dump_buf[MAX_GLOBAL_SEND_BUF + sizeof(EXTERN_DATA)];
		memcpy(dump_buf, head, ENDION_FUNC_4(head->len));
		PROTO_HEAD *head1 = (PROTO_HEAD *)dump_buf;
		EXTERN_DATA ext_data;
		ext_data.open_id = open_id;
		ext_data.player_id = player_id;
		ext_data.fd = fd;
		ext_data.port = sock.sin_port;
		add_extern_data(head1, &ext_data);
		transfer_to_dumpserver(head1);
	}
#if 1

#ifdef FLOW_MONITOR
	add_one_client_answer(head);
#endif

//	static int seq = 1;
	char *p = (char *)head;
	int len = ENDION_FUNC_4(head->len);
//	head->seq = ENDION_FUNC_2(seq++);

	if (send_buffer_end_pos+len >= MAX_SEND_BUFFER_SIZE) {  ///缓冲区溢出, 关闭连接
		LOG_DEBUG("[%s: %d]: fd: %d: send msg[%d] len[%d], seq[%d] buffer full", __PRETTY_FUNCTION__, __LINE__, fd, ENDION_FUNC_2(head->msg_id), ENDION_FUNC_4(head->len), ENDION_FUNC_2(head->seq));
		return -1;
	}


	if (head->msg_id != MSG_ID_HEARTBEAT_NOTIFY)
		LOG_DEBUG("[%s: %d]: fd: %d: send msg[%d] len[%d], seq[%d]", __PRETTY_FUNCTION__, __LINE__, fd, ENDION_FUNC_2(head->msg_id), ENDION_FUNC_4(head->len), ENDION_FUNC_2(head->seq));

	memcpy(send_buffer+send_buffer_end_pos, p, len);
	encoder_data((PROTO_HEAD*)(send_buffer+send_buffer_end_pos));

	send_buffer_end_pos += len;

	if (send_buffer_begin_pos == 0) {
		int result = event_add(&this->ev_write, NULL);
		if (0 != result) {
			LOG_ERR("[%s : %d]: event add failed, result: %d", __PRETTY_FUNCTION__, __LINE__, result);
			return result;
		}
	}

	return 0;
#else
	return conn_node_base::send_one_msg(head, force);
#endif
}

int conn_node_client::dispatch_message()
{
	PROTO_HEAD *head = (PROTO_HEAD *)buf_head();

	uint32_t cmd = ENDION_FUNC_2(head->msg_id);


	switch (cmd)
	{
		case MSG_ID_PLAYER_LIST_REQUEST:
		{
			if (player_id > 0)
			{
				return send_player_exit(true);
			}
			else
			{
				return transfer_to_loginserver();
			}
		}
		break;
		case MSG_ID_MAIL_LIST_REQUEST:
		case MSG_ID_MAIL_READ_REQUEST:
		case MSG_ID_MAIL_GET_ATTACH_REQUEST:
		case MSG_ID_MAIL_DEL_REQUEST:
			transfer_to_mailsrv();
			break;
		case MSG_ID_LIST_WANYAOKA_REQUEST:
		case MSG_ID_FRIEND_INFO_REQUEST:
		case MSG_ID_FRIEND_ADD_CONTACT_REQUEST:
		case MSG_ID_FRIEND_DEL_CONTACT_REQUEST:
		case MSG_ID_FRIEND_ADD_BLOCK_REQUEST:
		case MSG_ID_FRIEND_DEL_BLOCK_REQUEST:
		case MSG_ID_FRIEND_DEL_ENEMY_REQUEST:
		case MSG_ID_FRIEND_CREATE_GROUP_REQUEST:
		case MSG_ID_FRIEND_EDIT_GROUP_REQUEST:
		case MSG_ID_FRIEND_REMOVE_GROUP_REQUEST:
		case MSG_ID_FRIEND_MOVE_PLAYER_GROUP_REQUEST:
		case MSG_ID_FRIEND_DEAL_APPLY_REQUEST:
//		case MSG_ID_FRIEND_RECOMMEND_REQUEST:
		case MSG_ID_FRIEND_SEND_GIFT_REQUEST:
			transfer_to_friendsrv();
			break;
		case MSG_ID_GUILD_LIST_REQUEST:
		case MSG_ID_GUILD_INFO_REQUEST:
		case MSG_ID_GUILD_MEMBER_LIST_REQUEST:
		case MSG_ID_GUILD_CREATE_REQUEST:
		case MSG_ID_GUILD_JOIN_REQUEST:
		case MSG_ID_GUILD_JOIN_LIST_REQUEST:
		case MSG_ID_GUILD_DEAL_JOIN_REQUEST:
		case MSG_ID_GUILD_TURN_SWITCH_REQUEST:
		case MSG_ID_GUILD_SET_WORDS_REQUEST:
		case MSG_ID_GUILD_APPOINT_OFFICE_REQUEST:
		case MSG_ID_GUILD_KICK_REQUEST:
		case MSG_ID_GUILD_RENAME_REQUEST:
		case MSG_ID_GUILD_EXIT_REQUEST:
		case MSG_ID_GUILD_BUILDING_INFO_REQUEST:
		case MSG_ID_GUILD_BUILDING_UPGRADE_REQUEST:
		case MSG_ID_GUILD_SKILL_INFO_REQUEST:
		case MSG_ID_GUILD_SKILL_DEVELOP_REQUEST:
		case MSG_ID_GUILD_SKILL_PRACTICE_REQUEST:
		case MSG_ID_GUILD_SHOP_INFO_REQUEST:
		case MSG_ID_GUILD_SHOP_BUY_REQUEST:
		case MSG_ID_OPEN_FACTION_QUESTION_REQUEST:
		case MSG_ID_GUILD_BATTLE_CALL_REQUEST:
		case MSG_ID_GUILD_SET_PERMISSION_REQUEST:
		case MSG_ID_GUILD_ACCEPT_TASK_REQUEST:
		case MSG_ID_GUILD_INVITE_REQUEST:
		case MSG_ID_GUILD_DEAL_INVITE_REQUEST:
		case MSG_ID_GUILD_DONATE_REQUEST:
			return transfer_to_guildsrv();
		case MSG_ID_RANK_INFO_REQUEST:
		case MSG_ID_WORLDBOSS_REAL_RANK_INFO_REQUEST:
		case MSG_ID_WORLDBOSS_ZHUJIEMIAN_INFO_REQUEST:
		case MSG_ID_WORLDBOSS_LAST_RANK_INFO_REQUEST:
			return transfer_to_ranksrv();
		case MSG_ID_DOUFACHANG_CHALLENGE_REQUEST:
		case MSG_ID_DOUFACHANG_INFO_REQUEST:
		case MSG_ID_DOUFACHANG_GET_REWARD_REQUEST:
		case MSG_ID_DOUFACHANG_RECORD_REQUEST:
		case MSG_ID_DOUFACHANG_BUY_CHALLENGE_REQUEST:
			return transfer_to_doufachang();
		case MSG_ID_TRADE_OFF_SHELF_REQUEST:
		case MSG_ID_TRADE_RESHELF_REQUEST:
		case MSG_ID_TRADE_ENLARGE_SHELF_REQUEST:
		case MSG_ID_TRADE_ITEM_SUMMARY_REQUEST:
		case MSG_ID_TRADE_ITEM_DETAIL_REQUEST:
		case MSG_ID_TRADE_BUY_REQUEST:
		case MSG_ID_TRADE_GET_EARNING_REQUEST:
		case MSG_ID_AUCTION_BID_REQUEST:
		case MSG_ID_AUCTION_BUY_NOW_REQUEST:
		case MSG_ID_AUCTION_INFO_REQUEST:
			return transfer_to_tradesrv();
		case MSG_ID_ZHANLIDAREN_GET_REWARD_REQUEST:
			return transfer_to_activitysrv();
		default:
			return transfer_to_gameserver();
	}

	return (0);
}

int conn_node_client::transfer_to_raidserver()
{
	PROTO_HEAD *head;
	head = (PROTO_HEAD *)buf_head();
	uint32_t cmd = ENDION_FUNC_2(head->msg_id);
	UNUSED(cmd);
	switch (cmd)
	{
		case MSG_ID_MOVE_REQUEST:
		case MSG_ID_ENTER_SCENE_READY_REQUEST:
		case MSG_ID_MOVE_START_REQUEST:
		case MSG_ID_MOVE_STOP_REQUEST:
		case MSG_ID_MOVE_Y_START_REQUEST:
		case MSG_ID_MOVE_Y_STOP_REQUEST:
		case MSG_ID_SKILL_CAST_REQUEST:
		case MSG_ID_SKILL_HIT_REQUEST:
		case MSG_ID_PARTNER_SKILL_CAST_REQUEST:
		case MSG_ID_TRANSFER_OUT_STUCK_REQUEST:
		case MSG_ID_RELIVE_REQUEST:
		case MSG_ID_COLLECT_BEGIN_REQUEST:
		case MSG_ID_COLLECT_INTERRUPT_REQUEST:
		case MSG_ID_COLLECT_COMMPLETE_REQUEST:
		case MSG_ID_SING_INTERRUPT_REQUEST:
		case MSG_ID_SING_END_REQUEST:
		case MSG_ID_SING_BEGIN_REQUEST:
		case MSG_ID_TRANSFER_TO_LEADER_REQUEST:
		case MSG_ID_LEAVE_RAID_REQUEST:
		case MSG_ID_TEAM_RAID_READY_REQUEST:
		case MSG_ID_TEAM_RAID_CANCEL_REQUEST:
		case MSG_ID_NPC_TALK_REQUEST:
		case MSG_ID_RAID_AI_CONTINUE_REQUEST:
			break;
		default:
			return (-1);
	}
	
	if (!conn_node_raidsrv::server_node[0]) {
		return (-1);
	}

	if (conn_node_raidsrv::server_node[0]->send_one_msg(head, 1) != (int)(ENDION_FUNC_4(head->len))) {
		LOG_ERR("%s: send to raidserver failed err[%d]", __PRETTY_FUNCTION__, errno);
		return (-10);
	}
	return (0);
}

int conn_node_client::transfer_to_gameserver()
{
	if (raidsrv_id >= 0 && transfer_to_raidserver() == 0)
	{
		return (0);
	}
	
	int ret = 0;
	PROTO_HEAD *head;
	head = (PROTO_HEAD *)buf_head();

	if (!conn_node_gamesrv::server_node) {
		if (conn_node_gamesrv::add_cached_buf(head) != 0) {
			LOG_ERR("%s %d: add cached buf failed", __PRETTY_FUNCTION__, __LINE__);
		}
//		LOG_ERR("%s: do not have game server connected", __PRETTY_FUNCTION__);
		ret = -1;
		goto done;
	}

	if (conn_node_gamesrv::server_node->send_one_msg(head, 1) != (int)(ENDION_FUNC_4(head->len))) {
		LOG_ERR("%s: send to gameserver failed err[%d]", __PRETTY_FUNCTION__, errno);
		ret = -2;
		goto done;
	}
done:
	return (ret);
}

int conn_node_client::transfer_to_loginserver()
{
	int ret = 0;
	PROTO_HEAD *head;
	head = (PROTO_HEAD *)buf_head();

	uint32_t cmd = ENDION_FUNC_2(head->msg_id);

	conn_node_login *server = conn_node_login::get_normal_node();
	if (!server) {
		LOG_ERR("[%s:%d] do not have login server connected", __FUNCTION__, __LINE__);
		ret = -1;
		goto done;
	}

	if (cmd == MSG_ID_LOGIN_REQUEST) {
		server->locker();
		//head->seq = login_seq;
		login_seq = head->seq;
	}

	if (server->send_one_msg(head, 1) != (int)ENDION_FUNC_4(head->len)) {
		LOG_ERR("[%s:%d] send to loginserver failed err[%d]", __FUNCTION__, __LINE__, errno);
		ret = -2;
		goto done;
	}
done:
	return (ret);
}

int conn_node_client::transfer_to_dumpserver(PROTO_HEAD *head)
{
	int ret = 0;
//	PROTO_HEAD *head;
//	head = (PROTO_HEAD *)buf_head();
//	EXTERN_DATA *extern_data = get_extern_data(head);

//	uint32_t cmd = ENDION_FUNC_2(head->msg_id);
//	LOG_DEBUG("[%s:%d] cmd: %u, open_id: %u, player_id: %lu, fd: %u, port: %u", __FUNCTION__, __LINE__, cmd, extern_data->open_id, extern_data->player_id, extern_data->fd, extern_data->port);

	conn_node_dump *server = conn_node_dump::server_node;
	if (!server) {
//		LOG_ERR("[%s:%d] do not have login server connected", __FUNCTION__, __LINE__);
		ret = -1;
		goto done;
	}

	if (!(server->player_id == 0 || player_id == 0 || server->player_id == player_id)) {
		ret = 0;
		goto done;
	}

	if (server->send_one_msg(head, 1) != (int)ENDION_FUNC_4(head->len)) {
		LOG_ERR("[%s:%d] send to dumpserver failed err[%d]", __FUNCTION__, __LINE__, errno);
		ret = -2;
		goto done;
	}


#ifdef FLOW_MONITOR
	add_on_other_server_answer_msg(head);
#endif


done:
	return (ret);
}

int conn_node_client::transfer_to_mailsrv()
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
done:
	return (ret);
}
int conn_node_client::transfer_to_friendsrv()
{
	int ret = 0;
	PROTO_HEAD *head;
	head = (PROTO_HEAD *)buf_head();

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
done:
	return (ret);
}

int conn_node_client::transfer_to_guildsrv()
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
done:
	return (ret);
}

int conn_node_client::transfer_to_ranksrv()
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
done:
	return (ret);
}

int conn_node_client::transfer_to_doufachang()
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
done:
	return (ret);
}

int conn_node_client::transfer_to_tradesrv()
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
done:	
	return (ret);
}

int conn_node_client::transfer_to_activitysrv()
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

//////////////////////////// 下面是static 函数
std::map<evutil_socket_t, conn_node_client *> conn_node_client::map_fd_nodes;
std::map<uint64_t, conn_node_client *> conn_node_client::map_player_id_nodes;
std::map<uint32_t, conn_node_client *> conn_node_client::map_open_id_nodes;

int conn_node_client::add_map_fd_nodes(conn_node_client *client)
{
	map_fd_nodes[client->fd]=  client;
	return (0);
}
int conn_node_client::add_map_player_id_nodes(conn_node_client *client)
{
		//todo check duplicate
	if (client->player_id > 0)
		map_player_id_nodes[client->player_id] = client;
	return (0);
}

int conn_node_client::add_map_open_id_nodes(conn_node_client *client)
{
		//todo check duplicate
	if (client->open_id > 0)
		map_open_id_nodes[client->open_id] = client;
	return (0);
}

conn_node_client *conn_node_client::get_nodes_by_fd(evutil_socket_t fd, uint16_t port)
{
	std::map<evutil_socket_t, conn_node_client *>::iterator it = map_fd_nodes.find(fd);
	if (it != map_fd_nodes.end() && it->second->sock.sin_port == port)
		return it->second;
	return NULL;
}

conn_node_client *conn_node_client::get_nodes_by_player_id(uint64_t player_id)
{
	std::map<uint64_t, conn_node_client *>::iterator it = map_player_id_nodes.find(player_id);
	if (it != map_player_id_nodes.end())
		return it->second;
	return NULL;
}

conn_node_client *conn_node_client::get_nodes_by_open_id(uint32_t open_id)
{
	std::map<uint32_t, conn_node_client *>::iterator it = map_open_id_nodes.find(open_id);
	if (it != map_open_id_nodes.end())
		return it->second;
	return NULL;
}


void on_write(int fd, short ev, void *arg) {
	assert(arg);
	conn_node_client *client = (conn_node_client *)arg;
	client->send_data_to_client();
}

void conn_node_client::send_data_to_client() {
	if (send_buffer_end_pos-send_buffer_begin_pos<=0)
		return;

	int len = write(this->fd, send_buffer + send_buffer_begin_pos, send_buffer_end_pos-send_buffer_begin_pos);

	LOG_DEBUG("%s %d: write to fd[%u] ret %d, end pos = %d, begin pos = %d", __PRETTY_FUNCTION__, __LINE__, fd, len, send_buffer_end_pos, send_buffer_begin_pos);

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

	if (send_buffer_end_pos>=MAX_SEND_BUFFER_SIZE/2 && (send_buffer_begin_pos/1024) > 0 && send_buffer_end_pos>send_buffer_begin_pos) {
		int sz = send_buffer_end_pos - send_buffer_begin_pos;
		memmove(send_buffer, send_buffer+send_buffer_begin_pos, sz);
		send_buffer_begin_pos = 0;
		send_buffer_end_pos = sz;
	}
}
