//接收来自login srv的消息
#include "conn_node_login.h"
#include "conn_node_client.h"
#include "conn_node_gamesrv.h"
//#include "conn_node_guild.h"
//#include "conn_node_logger.h"
#include "msgid.h"
#include "login.pb-c.h"
#include "game_event.h"
#include "server_proto.h"
#include "error_code.h"
#include "flow_record.h"
#include <assert.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

extern uint32_t sg_server_id;

conn_node_login::conn_node_login()
{
	max_buf_len = 1024 * 1024;
	buf = (uint8_t *)malloc(max_buf_len + sizeof(EXTERN_DATA));
	assert(buf);
}

conn_node_login::~conn_node_login()
{
	server_node[m_id] = NULL;
}

int conn_node_login::recv_func(evutil_socket_t fd)
{
	for (;;) {
		int ret = get_one_buf();
		if (ret == 0) {
			if (transfer_to_client() != 0) {
				LOG_ERR("%s %d: fd[%d] transfer_to_client failed err = %d", __FUNCTION__, __LINE__, fd, errno);
	//			remove_one_buf();
	//			return (0);
			}
		}
		
		if (ret < 0) {
			LOG_ERR("%s %d: connect closed from fd %u, err = %d", __FUNCTION__, __LINE__, fd, errno);
			return (-1);		
		} else if (ret > 0) {
			break;
		}
		
		ret = remove_one_buf();
	}
	return (0);
}

int conn_node_login::transfer_to_gameserver()
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

int conn_node_login::handle_client_enter(EXTERN_DATA *extern_data)
{
//	PROTO_ROLE_ENTER *proto = (PROTO_ROLE_ENTER *)buf_head();
	conn_node_client *client = conn_node_client::get_nodes_by_open_id(extern_data->open_id);
	if (!client) {
			//客户端已经断开?
		LOG_INFO("%s %d: can not find openid[%u]", __FUNCTION__, __LINE__, extern_data->open_id);
		return (0);
	}
	if (client->player_id > 0)
	{
		LOG_ERR("%s %d: openid[%u] playerid[%lu] already in game", __FUNCTION__, __LINE__, extern_data->open_id, client->player_id);
		return (0);		
	}
	
	//记录player_id，绑定到map，此后client来的消息由transfer_to_gameserver处理
	uint64_t player_id = extern_data->player_id;
	client->player_id = player_id;
	client->raidsrv_id = -1;
	conn_node_client::add_map_player_id_nodes(client);

	LOG_DEBUG("[%s:%d] openid[%u] playerid[%lu] fd[%u]", __FUNCTION__, __LINE__, client->open_id, client->player_id, client->fd);
	
	return transfer_to_gameserver();
}

int conn_node_login::handle_client_login_success(EXTERN_DATA *extern_data, uint16_t seq)
{
	conn_node_client *client;
	client = conn_node_client::get_nodes_by_fd(extern_data->fd, extern_data->port);
	if (!client || client->login_seq != (seq))
	{
		int client_seq = 0;
		if (client)
			client_seq = client->login_seq;
		LOG_ERR("%s %d: client[%p] fd[%u] seq[%u] client_seq[%u]", __FUNCTION__, __LINE__, client, extern_data->fd, (seq), client_seq);
		return (0);
	}
		
	if (extern_data->open_id > 0) {
		client = conn_node_client::get_nodes_by_open_id(extern_data->open_id);
		if (client) {
			char buf[32];
			PROTO_HEAD *head = (PROTO_HEAD *)buf;
			head->msg_id = ENDION_FUNC_2(MSG_ID_ACCOUNT_LOGIN_AGAIN_NOTIFY);
			head->seq = 0;
			head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD));
			int ret = write(client->fd, head, (sizeof(PROTO_HEAD)));
//			client->send_one_msg(head, 1);
			LOG_INFO("%s %d: kick old openid[%u] fd[%u], ret[%d]", __FUNCTION__, __LINE__, client->open_id, client->fd, ret);
			remove_listen_callback_event(client);
		}
	}
		
	client = conn_node_client::get_nodes_by_fd(extern_data->fd, extern_data->port);
	if (!client || client->open_id || client->player_id) {
		uint64_t player_id = 0;
		if (client)
			player_id = client->player_id;
		LOG_ERR("[%s: %d]:  client error, fd: %d, open id: %u, player id: %lu", __FUNCTION__, __LINE__,
			extern_data->fd, extern_data->open_id, player_id);
		return (-1);
	}
	
	//记录open_id，绑定到map
	client->open_id = extern_data->open_id;
	conn_node_client::add_map_open_id_nodes(client);

	LOG_DEBUG("%s %d: openid[%u] fd[%u]", __FUNCTION__, __LINE__, extern_data->open_id, extern_data->fd);
	
	LoginAnswer resp;	
	login_answer__init(&resp);
	resp.result = 0;
	resp.openid = extern_data->open_id;
	
	char buf[256];
	PROTO_HEAD *head = (PROTO_HEAD *)buf;
	head->msg_id = ENDION_FUNC_2(MSG_ID_LOGIN_ANSWER);
	head->seq = get_seq();
	size_t size = login_answer__pack(&resp, (uint8_t *)head->data);
	head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD) + size);

	int ret = client->send_one_msg(head, 1);
	if (ret != 0) {
		LOG_ERR("%s: send to client failed err[%d]", __FUNCTION__, errno);
		return (0);
	}

	return (0);
}
int conn_node_login::handle_client_login_fail(EXTERN_DATA *extern_data, int result, uint16_t seq)
{
	conn_node_client *client;
	
	client = conn_node_client::get_nodes_by_fd(extern_data->fd, extern_data->port);
	if (!client || client->open_id || client->player_id || client->login_seq != seq) {
		uint64_t player_id = 0;
		if (client)
			player_id = client->player_id;
		LOG_ERR("[%s: %d]:  client error, fd: %d, open id: %u, player id: %lu", __FUNCTION__, __LINE__,
			extern_data->fd, extern_data->open_id, player_id);
		return (-1);
	}
	
	LOG_DEBUG("%s %d: openid[%u] fd[%u]", __FUNCTION__, __LINE__, extern_data->open_id, extern_data->fd);
	
	LoginAnswer resp;
	login_answer__init(&resp);
	resp.result = result;
	resp.openid = extern_data->open_id;
	
	char buf[256];
	PROTO_HEAD *head = (PROTO_HEAD *)buf;
	head->msg_id = ENDION_FUNC_2(MSG_ID_LOGIN_ANSWER);
	head->seq = get_seq();
	size_t size = login_answer__pack(&resp, (uint8_t *)head->data);
	head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD) + size);

	int ret = client->send_one_msg(head, 1);
	if (ret != 0) {
		LOG_ERR("%s: send to client failed err[%d]", __FUNCTION__, errno);
		return (0);
	}
	return (0);
}

int conn_node_login::handle_client_login(EXTERN_DATA *extern_data)
{
	PROTO_ROLE_LOGIN *proto = (PROTO_ROLE_LOGIN *)buf_head();

	if (proto->result == 0)
	{
		return handle_client_login_success(extern_data, (proto->login_seq));
	}
	else
	{
		return handle_client_login_fail(extern_data, proto->result, (proto->login_seq));		
	}

}

int conn_node_login::transfer_to_client()
{
	uint32_t old_len;
	PROTO_HEAD *head;
	conn_node_client *client;
	EXTERN_DATA *extern_data;

	head = (PROTO_HEAD *)get_head();
	extern_data = get_extern_data(head);
	int cmd = ENDION_FUNC_2(head->msg_id);
 	if (cmd ==SERVER_PROTO_LOGIN) {
 		this->unlocker();
 	}

#ifdef FLOW_MONITOR
	add_one_other_server_request_msg(head);
#endif

	switch (cmd)
	{
		case SERVER_PROTO_LOGIN:
			return handle_client_login(extern_data);
//		case SERVER_PROTO_TIREN_LIST_NOTIFY:
//			return handle_tiren();
//		case SERVER_PROTO_ENTER:
		case SERVER_PROTO_ENTER_GAME_REQUEST:
//			transfer_to_guildserver();			
			return handle_client_enter(extern_data);
/*		case SERVER_PROTO_GET_PLAYER_INFO_ANSWER:*/
// 		case SERVER_PROTO_GET_USER_DETAIL_ANSWER:
// 			return transfer_to_gameserver();
//		case SERVER_PROTO_GET_PLAYER_INFO_ANSWER:
//			return transfer_to_guildserver();
//		case SERVER_PROTO_STATIS_INFO:
//			return transfer_to_loggersrv();
	}
	
	old_len = ENDION_FUNC_4(head->len);

	client = conn_node_client::get_nodes_by_fd(extern_data->fd, extern_data->port);	
	if (!client) {
		LOG_ERR("%s %d: can not find client[%u] [%u], id = %d", __PRETTY_FUNCTION__, __LINE__, extern_data->fd, extern_data->port, cmd);		
//		LOG_ERR("%s %d: can not find client[%u]", __FUNCTION__, __LINE__, extern_data->fd);
		return (0);		
	}

	if (old_len < sizeof(PROTO_HEAD) + sizeof(EXTERN_DATA)) {
		LOG_ERR("%s %d: len wrong drop all msg", __FUNCTION__, __LINE__);
		return (-1);
	}
	head->len = ENDION_FUNC_4(old_len - sizeof(EXTERN_DATA));

	int ret = client->send_one_msg(head, 1);
	if (ret != 0) {
//	if (ret != htons(head->len)) {
		LOG_ERR("%s %d: send to client[%d] failed err[%d]", __FUNCTION__, __LINE__, extern_data->fd, errno);
//		return (0);
	}
	head->len = ENDION_FUNC_4(old_len);
	return (0);
}

int conn_node_login::kick_role(int fd)
{
	return (0);
}

int conn_node_login::transfer_to_guildserver() {
	int ret = 0;
/*	PROTO_HEAD *head;
	head = (PROTO_HEAD *)buf_head();

	if (!conn_node_guild::server_node) {
		ret = -1;
		goto done;
	}

	if (conn_node_guild::server_node->send_one_msg(head, 1) != (int)get_real_head_len(head)) {		
		LOG_ERR("%s: send to gameserver failed err[%d]", __FUNCTION__, errno);
		ret = -2;
		goto done;
	}
done:	*/
	return (ret);
}

/*int conn_node_login::handle_tiren() {
	PLAYER_TIREN_LIST_NOTIFY* head = (PLAYER_TIREN_LIST_NOTIFY *)buf_head();

	for (uint32_t i=0; i<head->count; ++i) {
		uint32_t open_id = head->open_id[i];
		if (0 == open_id)
			continue;

		conn_node_client* client = conn_node_client::get_nodes_by_open_id(open_id);
		if (client) {
			char buf[32];
			PROTO_HEAD *head = (PROTO_HEAD *)buf;
			head->msg_id = htons(ERRORCODE_PLAYER_FENG_HAO);
			head->seq = 0;
			head->len = htons(sizeof(PROTO_HEAD));
			send(client->fd, head, (sizeof(PROTO_HEAD)), 0);
			//			client->send_one_msg(head, 1);
			LOG_INFO("%s %d: kick old openid[%u] fd[%u]", __FUNCTION__, __LINE__, client->open_id, client->fd);
			remove_listen_callback_event(client);
		}		
	}

	return 0;
}*/

int conn_node_login::transfer_to_loggersrv() {
	int ret = 0;
/*	PROTO_HEAD *head;
	head = (PROTO_HEAD *)buf_head();

	PROTO_STATIS_INFO* statisHeader = (PROTO_STATIS_INFO*)head;
	conn_node_client* cli = conn_node_client::get_nodes_by_player_id(statisHeader->player_id);
	if (cli) {
		statisHeader->open_id = cli->open_id;
		if (statisHeader->server_id == 0) {
			statisHeader->server_id = sg_server_id;
		}
	}

	if (!conn_node_logger::server_node) {
		LOG_ERR("%s: do not have active server connected", __FUNCTION__);
		ret = -1;
		goto done;
	}

	if (conn_node_logger::server_node->send_one_msg(head, 1) != htons(head->len)) {		
		LOG_ERR("%s: send to active server failed err[%d]", __FUNCTION__, errno);
		ret = -2;
		goto done;
	}
done:	*/
	return (ret);
}

void conn_node_login::setid(int id)
{
	m_id = id;
}

conn_node_login *conn_node_login::get_normal_node()
{
	for (int i = 0; i < MAX_LOGIN_SERVER_NUM; ++i)
	{
		if (conn_node_login::server_node[i] && conn_node_login::server_node[i]->state == LOGIN_SERVER_STATE_NORMAL)
			return conn_node_login::server_node[i];
	}
	return NULL;
}

void conn_node_login::locker()
{
	LOG_DEBUG("%s: id = [%d]", __FUNCTION__, m_id);	
	state = LOGIN_SERVER_STATE_LOCK;
}

void conn_node_login::unlocker()
{
	LOG_DEBUG("%s: id = [%d]", __FUNCTION__, m_id);	
	state = LOGIN_SERVER_STATE_NORMAL;
}

int conn_node_login::send_one_msg(PROTO_HEAD *head, uint8_t force)
{
	static int seq = 1;
	int ret;
	int send_num = 0;
	char *p = (char *)head;
	int len = ENDION_FUNC_4(head->len);

	if (ENDION_FUNC_2(head->msg_id) != MSG_ID_LOGIN_REQUEST)
		head->seq = ENDION_FUNC_2(seq++);

	for (;;) {
		ret = send(fd, p, len, 0);
		assert(ret <= len);
		if (ret == len)
			goto done;
		if (ret < 0) {
				//force可能会导致bug，如果一个数据包发送了一半然后发生eagain，这个时候如果失败可能导致后续的数据包错误
			if (errno != EAGAIN)// || force)  
				goto fail;
				//ignore EINPROGRESS
			usleep(100000);			
		} else if (ret < len) {
			len -= ret;
			p += ret;
			send_num += ret;
			usleep(100000);
		}
	}
done:
	if (head->msg_id != 0)
		LOG_DEBUG("%s %d: send msg[%d] len[%d], seq[%d], ret [%d]", __FUNCTION__, fd, ENDION_FUNC_2(head->msg_id), ENDION_FUNC_4(head->len), ENDION_FUNC_2(head->seq), ret);
#ifdef CALC_NET_MSG
	uint16_t id = htons(head->msg_id);
	if (id < CS__MESSAGE__ID__MAX_MSG_ID) {
		send_buf_size[id] += len;
		++send_buf_times[id];
	}
#endif	
	
	return (ENDION_FUNC_4(head->len));
fail:
	LOG_ERR("%s fd[%d]: msg[%d] len[%d] seq[%d] ret[%d] errno[%d] send_num = %d",
		__FUNCTION__, fd, ENDION_FUNC_2(head->msg_id), ENDION_FUNC_4(head->len), ENDION_FUNC_2(head->seq), ret, errno, send_num);
	return ret;
}

//////////////////////////// 下面是static 函数
conn_node_login *conn_node_login::server_node[MAX_LOGIN_SERVER_NUM];
