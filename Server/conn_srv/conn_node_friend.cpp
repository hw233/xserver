//接收来自friend srv的消息
#include "conn_node_friend.h"
#include "conn_node_client.h"
#include "conn_node_gamesrv.h"
#include "conn_node_mail.h"
#include "msgid.h"
#include "game_event.h"
#include "server_proto.h"
#include "error_code.h"
#include "flow_record.h"
#include <assert.h>
#include <errno.h>
#include <unistd.h>

extern uint32_t sg_server_id;


conn_node_friend::conn_node_friend()
{
	max_buf_len = 1024 * 1024;
	buf = (uint8_t *)malloc(max_buf_len + sizeof(EXTERN_DATA));
	assert(buf);
}

conn_node_friend::~conn_node_friend()
{
	server_node = NULL;
}

int conn_node_friend::recv_func(evutil_socket_t fd)
{
	for (;;) {
		int ret = get_one_buf();
		if (ret == 0) {
			if (transfer_to_client() != 0) {
				LOG_ERR("%s %d: fd[%d] transfer_to_client failed err = %d", __FUNCTION__, __LINE__, fd, errno);
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

int conn_node_friend::friend_to_gameserver(PROTO_HEAD *head)
{
	int ret = 0;

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

int conn_node_friend::transfer_to_mailsrv()
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

int conn_node_friend::transfer_to_gameserver()
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

int conn_node_friend::broadcast_to_all_client()
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

int conn_node_friend::transfer_to_client()
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
	case SERVER_PROTO_MAIL_INSERT:
		return transfer_to_mailsrv();
		case SERVER_PROTO_BROADCAST:
			return broadcast_to_client();
		case SERVER_PROTO_GET_OFFLINE_CACHE_ANSWER:
		case SERVER_PROTO_FRIEND_EXTEND_CONTACT_ANSWER:
		case SERVER_PROTO_FRIENDSRV_COST_REQUEST:
		case SERVER_PROTO_FRIEND_GIFT_COST_REQUEST:
		case SERVER_PROTO_FRIEND_SYNC_FRIEND_NUM:
			return transfer_to_gameserver();
		case SERVER_PROTO_FRIEND_TO_GAME:
			return friend_to_gameserver((PROTO_HEAD *)head->data);
		case SERVER_PROTO_BROADCAST_ALL:
			return broadcast_to_all_client();
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
		//	if (ret != ENDION_FUNC_4(head->len)) {
		LOG_ERR("%s %d: send to client[%lu] failed err[%d]", __FUNCTION__, __LINE__, extern_data->player_id, errno);
		//		head->len = ENDION_FUNC_4(old_len);		
		//		return (0);
	}
	head->len = ENDION_FUNC_4(old_len);
	return (0);
}

int conn_node_friend::broadcast_to_client()
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

int conn_node_friend::send_one_msg(PROTO_HEAD *head, uint8_t force)
{
	static int seq = 1;
	int ret;
	int send_num = 0;
	char *p = (char *)head;
	int len = ENDION_FUNC_4(head->len);

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
conn_node_friend *conn_node_friend::server_node;
