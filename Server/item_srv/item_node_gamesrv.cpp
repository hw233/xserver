#include "item_node_gamesrv.h"
#include "conn_node_item.h"
#include "game_event.h"
#include <assert.h>
#include <errno.h>
#include "msgid.h"
#include "../proto/bag.pb-c.h"
#include "mysql_module.h"


item_node_gamesrv::item_node_gamesrv()
{
	max_buf_len = 1024 * 1024;
	buf = (uint8_t *)malloc(max_buf_len + sizeof(EXTERN_DATA));
	assert(buf);
}

item_node_gamesrv::~item_node_gamesrv()
{
	server_node = NULL;
}

int item_node_gamesrv::recv_func(evutil_socket_t fd)
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

/** game_srv与conn_srv链接处理 **/
int item_node_gamesrv::transfer_to_client()
{
	uint32_t old_len;
	PROTO_HEAD *head;
	EXTERN_DATA *extern_data;
	conn_node_item *client;

	head = (PROTO_HEAD *)get_head();

	int cmd = ENDION_FUNC_2(head->msg_id);
	

	extern_data = get_extern_data(head);
	LOG_DEBUG("[%s:%d]: Send Cmd To client, [%lu], cmd: %d", __FUNCTION__, __LINE__, extern_data->player_id, cmd);

 	client = conn_node_item::get_nodes_by_player_id(extern_data->player_id);
 	if (!client) {
 		LOG_ERR("%s %d: can not find client[%u] [%lu], id = %d", __PRETTY_FUNCTION__, __LINE__, extern_data->fd, extern_data->player_id, cmd);		
 		//return (0);		
 	}


	old_len = ENDION_FUNC_4(head->len);
	if (old_len < sizeof(PROTO_HEAD) + sizeof(EXTERN_DATA)) {	
		LOG_ERR("%s %d: len wrong drop all msg", __FUNCTION__, __LINE__);
		return (-1);
	}
	head->len = ENDION_FUNC_4(old_len - sizeof(EXTERN_DATA));

	show_item_to_itemsrv * pShow = (show_item_to_itemsrv *)buf_head();
	//todo 插入数据库
	uint64_t effect = 0;
	int len;
	char *p;
	char sql[1024 * 64];
	len = sprintf(sql, "insert into showitem set type=%u, data=\'" , pShow->type);
	p = sql + len;
	p += escape_string(p, (const char *)pShow->data, pShow->data_size);
	sprintf(p, "\'");

	query(const_cast<char*>("set names utf8"), 1, NULL);
	query(sql, 1, &effect);	

	sprintf(sql, "SELECT LAST_INSERT_ID()");

	MYSQL_RES *res = NULL;
	MYSQL_ROW row;
	res = query(sql, 1, NULL);
	if (!res) {
		return 1;
	}
	row = fetch_row(res);
	if (!row) {
		free_query(res);
		return 1;
	}

	ShowItemAnswer send;
	show_item_answer__init(&send);
	send.key = atol(row[0]);

	free_query(res);

	fast_send_msg(item_node_gamesrv::server_node, extern_data, 0, show_item_answer__pack, send);

	head->len = ENDION_FUNC_4(old_len);
	return (0);
}



int item_node_gamesrv::add_cached_buf(PROTO_HEAD *head)
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

int item_node_gamesrv::send_all_cached_buf()
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
//		if (item_node_gamesrv::server_node->send_one_msg(head, 1) != ENDION_FUNC_4(head->len)) {		
	return (0);

fail:
	if (pos != 0) {
		assert(cached_len <= pos);
		memmove(&cached_buf[0], &cached_buf[pos], cached_len - pos);
	}
	return (-1);
}


//////////////////////////// 下面是static 函数
item_node_gamesrv *item_node_gamesrv::server_node;
char item_node_gamesrv::cached_buf[1024 * 1024 * 1024];
int item_node_gamesrv::cached_len;
