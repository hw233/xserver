#include "conn_node_item.h"
#include "game_event.h"
#include "tea.h"
#include "msgid.h"
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

conn_node_item::conn_node_item()
{
	max_buf_len = 1024 * 1024;
	buf = (uint8_t *)malloc(max_buf_len + sizeof(EXTERN_DATA));
	assert(buf);

	open_id = 0;
	player_id = 0;

	send_buffer_begin_pos = 0;
	send_buffer_end_pos = 0;
}

conn_node_item::~conn_node_item()
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

int conn_node_item::send_player_exit(bool again/* = false*/)
{
	if (player_id == 0)
	{
		return 0;
	}

	//清除角色信息，放最后
	map_player_id_nodes.erase(player_id);
	player_id = 0;

	return 0;
}

int conn_node_item::decode_and_check_crc(PROTO_HEAD *head)
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

int conn_node_item::recv_func(evutil_socket_t fd)
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
				this->send_one_msg(head, 1);
			} else {				
				old_len = ENDION_FUNC_4(head->len);
				extern_data = (EXTERN_DATA *)&head->data[old_len - sizeof(PROTO_HEAD)];
				memcpy(&save_data, extern_data, sizeof(EXTERN_DATA));
				head->len = ENDION_FUNC_4(old_len + sizeof(EXTERN_DATA));				

				extern_data->player_id = player_id;
				extern_data->open_id = open_id;
				extern_data->fd = fd;
				extern_data->port = sock.sin_port;

				if (dispatch_message() != 0) {
				}

				head->len = ENDION_FUNC_4(old_len);
				memcpy(extern_data, &save_data, sizeof(EXTERN_DATA));
			}
		}
		
		if (ret < 0) {
			LOG_INFO("%s: connect closed from fd %u, err = %d", __PRETTY_FUNCTION__, fd, errno);		
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

int conn_node_item::send_one_msg(PROTO_HEAD *head, uint8_t force) {
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
	}
#if 1
//	static int seq = 1;
	char *p = (char *)head;
	int len = ENDION_FUNC_4(head->len);
//	head->seq = ENDION_FUNC_2(seq++);

	if (send_buffer_end_pos+len >= MAX_SEND_BUFFER_SIZE) {  ///缓冲区溢出, 关闭连接
		LOG_DEBUG("[%s: %d]: fd: %d: send msg[%d] len[%d], seq[%d] buffer full", __PRETTY_FUNCTION__, __LINE__, fd, ENDION_FUNC_2(head->msg_id), ENDION_FUNC_4(head->len), ENDION_FUNC_2(head->seq));
		return -1;
	} 


	if (head->msg_id != 0)
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

int conn_node_item::dispatch_message()
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
		
	}
		break;
	default:
		return 0;
	}

	return (0);
}



//////////////////////////// 下面是static 函数
std::map<evutil_socket_t, conn_node_item *> conn_node_item::map_fd_nodes;
std::map<uint64_t, conn_node_item *> conn_node_item::map_player_id_nodes;
std::map<uint32_t, conn_node_item *> conn_node_item::map_open_id_nodes;

int conn_node_item::add_map_fd_nodes(conn_node_item *client)
{
	map_fd_nodes[client->fd] = client;
	return (0);
}
int conn_node_item::add_map_player_id_nodes(conn_node_item *client)
{
		//todo check duplicate	
	map_player_id_nodes[client->player_id] = client;
	return (0);
}

int conn_node_item::add_map_open_id_nodes(conn_node_item *client)
{
		//todo check duplicate
	map_open_id_nodes[client->open_id] = client;
	return (0);
}

conn_node_item *conn_node_item::get_nodes_by_fd(evutil_socket_t fd, uint16_t port)
{
	std::map<evutil_socket_t, conn_node_item *>::iterator it = map_fd_nodes.find(fd);
	if (it != map_fd_nodes.end() && it->second->sock.sin_port == port)
		return it->second;
	return NULL;
}

conn_node_item *conn_node_item::get_nodes_by_player_id(uint64_t player_id)
{
	std::map<uint64_t, conn_node_item *>::iterator it = map_player_id_nodes.find(player_id);
	if (it != map_player_id_nodes.end())
		return it->second;
	return NULL;
}

conn_node_item *conn_node_item::get_nodes_by_open_id(uint32_t open_id)
{
	std::map<uint32_t, conn_node_item *>::iterator it = map_open_id_nodes.find(open_id);
	if (it != map_open_id_nodes.end())
		return it->second;
	return NULL;
}


void on_write(int fd, short ev, void *arg) {
	assert(arg);
	conn_node_item *client = (conn_node_item *)arg;	
	client->send_data_to_client();
}

void conn_node_item::send_data_to_client() {	
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

