#include "conn_node_dbsrv.h"
#include "game_event.h"
#include "error_code.h"
#include "mysql_module.h"
#include "msgid.h"
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string>
#include "../proto/role.pb-c.h"
#include "../proto/setting.pb-c.h"
#include "../proto/player_db.pb-c.h"
#include "../proto/friend.pb-c.h"
#include "../proto/personality.pb-c.h"

static char sql[1024 * 64];
//static char send_buf[1024 * 64];
static const uint32_t server_level_key = 1;
#define UNUSED(x) (void)(x)

conn_node_dbsrv::conn_node_dbsrv()
{
	max_buf_len = 1024 * 1024;
	buf = (uint8_t *)malloc(max_buf_len + sizeof(EXTERN_DATA));
	assert(buf);
}

conn_node_dbsrv::~conn_node_dbsrv()
{
	server_node = NULL;
}

void conn_node_dbsrv::handle_find_player(EXTERN_DATA *extern_data)
{
	PROTO_FIND_PLAYER_ANS *resp;
	PROTO_FIND_PLAYER_REQ *req = (PROTO_FIND_PLAYER_REQ *)buf_head();

	query(const_cast<char*>("set names utf8"), 1, NULL);

	uint64_t player_id = strtoull(req->name, NULL, 10);;
	sprintf(sql, "SELECT player_id, player_name, lv, chengjie_rest from player where player_id = %lu or player_name = \'%s\'", player_id,req->name);

	MYSQL_RES *res = query(sql, 1, NULL);
	MYSQL_ROW row;

	row = fetch_row(res);
	

	resp = (PROTO_FIND_PLAYER_ANS *)&conn_node_base::global_send_buf[0];
	resp->head.len = ENDION_FUNC_4(sizeof(PROTO_FIND_PLAYER_ANS));
	resp->head.msg_id = ENDION_FUNC_2(SERVER_PROTO_FIND_PLAYER_ANSWER);

	resp->player_id = 0;
	if (row != NULL)
	{
		strncpy(resp->name, row[1], MAX_PLAYER_NAME_LEN);
		resp->name[MAX_PLAYER_NAME_LEN - 1] = '\0';
		resp->lv = atoi(row[2]);
		resp->player_id = atoll(row[0]);
		resp->cd = atol(row[3]);
	}

	add_extern_data(&resp->head, extern_data);
	if (!server_node || server_node->send_one_msg(&resp->head, 1) != (int)ENDION_FUNC_4(resp->head.len)) {
		LOG_ERR("[%s:%d] send to gamesrv failed err[%d]", __FUNCTION__, __LINE__, errno);
	}
	
	free_query(res);
}

void conn_node_dbsrv::handle_save_player(EXTERN_DATA *extern_data)
{
	PROTO_SAVE_PLAYER_RESP *resp;
	PROTO_SAVE_PLAYER_REQ *req = (PROTO_SAVE_PLAYER_REQ *)buf_head();
	uint64_t effect = 0;
	int len;
	char *p;
	uint32_t result = ERROR_ID_SUCCESS;
	
	len = sprintf(sql, "update player set logout_time = now(), lv = %u,chengjie_rest=%lu, comm_data = \'", req->level,req->chengjie_cd);
	p = sql + len;
	p += escape_string(p, (const char *)req->data, req->data_size);
	len = sprintf(p, "\' where player_id = %lu", extern_data->player_id);

	LOG_DEBUG("[%s:%d] playerid: %lu, level: %u, comm_data size: %u.", __FUNCTION__, __LINE__, extern_data->player_id, req->level, req->data_size);
	query(sql, 1, &effect);	
	if (effect != 1) 
	{
		result = ERROR_ID_SAVE_DB;
		LOG_ERR("[%s:%d] save %lu failed", __FUNCTION__, __LINE__, extern_data->player_id);
		return;
	}
	resp = (PROTO_SAVE_PLAYER_RESP *)&conn_node_base::global_send_buf[0];
	resp->head.len = ENDION_FUNC_4(sizeof(PROTO_SAVE_PLAYER_RESP));
	resp->head.msg_id = ENDION_FUNC_2(SERVER_PROTO_SAVE_PLAYER);
	resp->again = req->again;
	resp->result = result;
	add_extern_data(&resp->head, extern_data);
	if (!server_node || server_node->send_one_msg(&resp->head, 1) != (int)ENDION_FUNC_4(resp->head.len)) {
		LOG_ERR("[%s:%d] send to gamesrv failed err[%d]", __FUNCTION__, __LINE__, errno);
	}
}

void conn_node_dbsrv::handle_add_chengjie(EXTERN_DATA *extern_data)
{
	PROTO_ADD_CHENGJIE_ANS *resp;
	PROTO_ADD_CHENGJIE *req = (PROTO_ADD_CHENGJIE *)buf_head();
	uint64_t effect = 0;
	int len;
	char *p;

	len = sprintf(sql, "insert into chengjie (data) values( \'");
	p = sql + len;
	p += escape_string(p, (const char *)req->data, req->data_size);
	len = sprintf(p, "\')");

	LOG_DEBUG("[%s:%d] playerid: %lu, comm_data size: %u.", __FUNCTION__, __LINE__, extern_data->player_id, req->data_size);
	query(sql, 1, &effect);
	if (effect != 1)
	{
		LOG_ERR("[%s:%d] save %lu failed", __FUNCTION__, __LINE__, extern_data->player_id);
		return;
	}
	
	
	resp = (PROTO_ADD_CHENGJIE_ANS *)&conn_node_base::global_send_buf[0];
	resp->head.len = ENDION_FUNC_4(sizeof(PROTO_ADD_CHENGJIE_ANS) + req->data_size);
	resp->head.msg_id = ENDION_FUNC_2(SERVER_PROTO_ADD_CHENGJIE_ANSWER);
	
	memcpy(resp->data, req->data, req->data_size);
	resp->data_size = req->data_size;

	sprintf(sql, "SELECT LAST_INSERT_ID()");
	MYSQL_RES *res = query(sql, 1, NULL);
	MYSQL_ROW row = fetch_row(res);
	if (row != NULL)
	{
		resp->taskid = atoll(row[0]);
	}

	add_extern_data(&resp->head, extern_data);
	if (!server_node || server_node->send_one_msg(&resp->head, 1) != (int)ENDION_FUNC_4(resp->head.len)) {
		LOG_ERR("[%s:%d] send to gamesrv failed err[%d]", __FUNCTION__, __LINE__, errno);
	}

	free_query(res);
}

void conn_node_dbsrv::handle_del_chengjie(EXTERN_DATA *extern_data)
{
	PROTO_CHENGJIE_ID *req = (PROTO_CHENGJIE_ID *)buf_head();
	uint64_t effect = 0;

	sprintf(sql, "delete from chengjie where id=%u", req->taskid);
	
	LOG_DEBUG("[%s:%d] playerid: %lu, comm_data size: %u.", __FUNCTION__, __LINE__, extern_data->player_id, 0);
	query(sql, 1, &effect);
	if (effect != 1)
	{
		LOG_ERR("[%s:%d] save %lu failed", __FUNCTION__, __LINE__, extern_data->player_id);
		return;
	}
}

void conn_node_dbsrv::handle_update_chengjie(EXTERN_DATA *extern_data)
{
	PROTO_ADD_CHENGJIE_ANS *req = (PROTO_ADD_CHENGJIE_ANS *)buf_head();
	uint64_t effect = 0;

	int len;
	char *p;
	len = sprintf(sql, "update chengjie set data = \'"); 
	p = sql + len;
	p += escape_string(p, (const char *)req->data, req->data_size);
	len = sprintf(p, "\' where id=%u", req->taskid);

	LOG_DEBUG("[%s:%d] playerid: %lu, comm_data size: %u.", __FUNCTION__, __LINE__, extern_data->player_id, 0);
	query(sql, 1, &effect);
	if (effect != 1)
	{
		LOG_ERR("[%s:%d] save %lu failed", __FUNCTION__, __LINE__, extern_data->player_id);
		return;
	}
}

void conn_node_dbsrv::handle_load_chengjie(EXTERN_DATA *extern_data)
{
	PROTO_ADD_CHENGJIE *resp;

	query(const_cast<char*>("set names utf8"), 1, NULL);

	sprintf(sql, "SELECT id, data from chengjie");

	MYSQL_RES *res = query(sql, 1, NULL);
	MYSQL_ROW row;

	row = fetch_row(res);

	unsigned long *lengths = mysql_fetch_lengths(res);
	resp = (PROTO_ADD_CHENGJIE *)&conn_node_base::global_send_buf[0];
	resp->head.len = ENDION_FUNC_4(sizeof(PROTO_ADD_CHENGJIE));
	resp->head.msg_id = ENDION_FUNC_2(SERVER_PROTO_LOAD_CHENGJIE_ANSWER);

	static const int MAX_SEND_CHENGJIE_TASK = 300;
	ChengjieTaskDb arr[MAX_SEND_CHENGJIE_TASK];
	ChengjieTaskDb *arrPoint[MAX_SEND_CHENGJIE_TASK];
	int sLen = 0;
	LoadChengjieTask send;
	load_chengjie_task__init(&send);
	send.data = arrPoint;
	add_extern_data(&resp->head, extern_data);
	while (row != NULL)
	{
		chengjie_task_db__init(arr + sLen);
		ChengjieTaskDb *tmp = chengjie_task_db__unpack(NULL, lengths[1], (const uint8_t*)row[1]);
		if (tmp == NULL)
		{
			row = fetch_row(res);
			lengths = mysql_fetch_lengths(res);
			continue;
		}
		arr[sLen].playerid = tmp->playerid ;
		arr[sLen].fail = tmp->fail;
		arr[sLen].shuangjin = tmp->shuangjin;
		arr[sLen].exp = tmp->exp;
		arr[sLen].courage = tmp->courage;
		arr[sLen].cd = tmp->cd;
		arr[sLen].complete = tmp->complete;
		arr[sLen].complete_cd = tmp->complete_cd;
		arr[sLen].accept_cd = tmp->accept_cd;
		arr[sLen].investor = tmp->investor;
		arr[sLen].accepter = tmp->accepter;
		arr[sLen].taskid = atol(row[0]);
		chengjie_task_db__free_unpacked(tmp, NULL);

		arrPoint[sLen] = arr + sLen;
		if (sLen > MAX_SEND_CHENGJIE_TASK)
		{
			send.n_data = sLen;
			resp->data_size = load_chengjie_task__pack(&send, resp->data);
			resp->head.len += resp->data_size;
			if (!server_node || server_node->send_one_msg(&resp->head, 1) != (int)ENDION_FUNC_4(resp->head.len)) {
				LOG_ERR("[%s:%d] send to gamesrv failed err[%d]", __FUNCTION__, __LINE__, errno);
			}
			sLen = 0;
		}
		else
		{
			++sLen;
		}

		row = fetch_row(res);
		lengths = mysql_fetch_lengths(res);
	}

	if (sLen > 0)
	{
		send.n_data = sLen;
		resp->data_size = load_chengjie_task__pack(&send, resp->data);
		resp->head.len += resp->data_size; 
		if (!server_node || server_node->send_one_msg(&resp->head, 1) != (int)ENDION_FUNC_4(resp->head.len)) {
			LOG_ERR("[%s:%d] send to gamesrv failed err[%d]", __FUNCTION__, __LINE__, errno);
		}
	}

	free_query(res);
}

int conn_node_dbsrv::recv_func(evutil_socket_t fd)
{
	EXTERN_DATA *extern_data;	
	PROTO_HEAD *head;
	for (;;) {
		int ret = get_one_buf();
		if (ret == 0) {
			head = (PROTO_HEAD *)buf_head();
			extern_data = get_extern_data(head);			
			switch (ENDION_FUNC_2(head->msg_id))
			{
				case SERVER_PROTO_SAVE_PLAYER:
					handle_save_player(extern_data);
					break;

				//case SERVER_PROTO_SAVA_CHAT_RECORD_REQUEST:
				//	handle_save_player_msg(extern_data);
				//	break;
				case SERVER_PROTO_ENTER_GAME_REQUEST:
					handle_enter_game(extern_data);
					break;
				case SERVER_PROTO_DOUFACHANG_LOAD_PLAYER_REQUEST:
					handle_doufachang_load(extern_data);
					break;
				case MSG_ID_PLAYER_RENAME_REQUEST:
					handle_rename_request(extern_data);
					break;
				case MSG_ID_SAVE_CLIENT_DATA_REQUEST:
					handle_save_client_data(extern_data);
					break;
				case MSG_ID_LOAD_CLIENT_DATA_REQUEST:
					handle_load_client_data(extern_data);
					break;
				case SERVER_PROTO_FIND_PLAYER_REQUEST:
					handle_find_player(extern_data);
					break;
				case SERVER_PROTO_ADD_CHENGJIE_REQUEST:
					handle_add_chengjie(extern_data);
					break;
				case SERVER_PROTO_DEL_CHENGJIE_REQUEST:
					handle_del_chengjie(extern_data);
					break;
				case SERVER_PROTO_UPDATE_CHENGJIE_REQUEST:
					handle_update_chengjie(extern_data);
					break;
				case SERVER_PROTO_LOAD_CHENGJIE_REQUEST:
					handle_load_chengjie(extern_data);
					break;
				case MSG_ID_FRIEND_SEARCH_REQUEST:
					handle_search_player(extern_data);
					break;
				case MSG_ID_GET_OTHER_INFO_REQUEST:
					handle_get_other_info(extern_data);
					break;
				case SERVER_PROTO_LOAD_SERVER_LEVEL_REQUEST:
					handle_load_server_level(extern_data);
					break;
				case SERVER_PROTO_SAVE_SERVER_LEVEL_REQUEST:
					handle_save_server_level(extern_data);
					break;
				case SERVER_PROTO_BREAK_SERVER_LEVEL_REQUEST:
					handle_break_server_level(extern_data);
					break;

				default:
					break;
			}
		}
		
		if (ret < 0) {
			LOG_INFO("%s %d: connect closed from fd %u, err = %d", __FUNCTION__, __LINE__, fd, errno);
			exit(0);
			return (-1);		
		} else if (ret > 0) {
			break;
		}
		
		ret = remove_one_buf();
	}
	return (0);
}

void conn_node_dbsrv::handle_save_player_msg(EXTERN_DATA *extern_data) {
/*	PLAYER_SAVA_CHAT_RECORD_REQUEST *req = (PLAYER_SAVA_CHAT_RECORD_REQUEST *)buf_head();

	uint64_t effect = 0;
	int len;
	char *p;

	len = sprintf(sql, "insert into player_chat_msg set `time`=now(), chat_type=%u, send_playerid=%.0f, playerid=%.0f, vip_level=%u, chat_name=\'"
		, req->chat_type, (double)req->chat_send_playerid, (double)req->chat_target_playerid, req->chat_send_vip_level);
	p = sql + len;
	p += escape_string(p, (const char *)req->chat_name, strlen(req->chat_name));
	len = sprintf(p, "\', chat_message=\'");	
	p += len;
	p += escape_string(p, (const char *)req->chat_message, strlen(req->chat_message));
	sprintf(p, "\'");

	query(const_cast<char*>("set names utf8"), 1, NULL);
	query(sql, 1, &effect);	*/
}

void conn_node_dbsrv::handle_doufachang_load(EXTERN_DATA *extern_data)
{
	DOUFACHANG_LOAD_PLAYER_REQUEST *req = (DOUFACHANG_LOAD_PLAYER_REQUEST *)conn_node_base::get_data();
	uint64_t player_id = req->target_id;

	unsigned long *lengths;	
	MYSQL_RES *res = NULL;
	MYSQL_ROW row;

	query(const_cast<char*>("set names utf8"), 1, NULL);

	sprintf(sql, "SELECT job, player_name, lv , comm_data, UNIX_TIMESTAMP(create_time), UNIX_TIMESTAMP(logout_time),chengjie_rest from player where player_id = %lu", player_id);
	res = query(sql, 1, NULL);
	if (!res) {
		LOG_ERR("[%s : %d]: query user failed, sql: %s", __FUNCTION__, __LINE__, sql);
		return;
	}
	
	row = fetch_row(res);
	if (!row) {
		LOG_ERR("[%s : %d]: query sql fetch row failed, sql: %s", __FUNCTION__, __LINE__, sql);
		free_query(res);
		return;
	}

	lengths = mysql_fetch_lengths(res);

	DOUFACHANG_LOAD_PLAYER_ANSWER *proto = (DOUFACHANG_LOAD_PLAYER_ANSWER *)server_node->get_send_data();
	proto->job = atoi(row[0]) & 0xff;
	strncpy(proto->name, row[1], MAX_PLAYER_NAME_LEN);
	proto->name[MAX_PLAYER_NAME_LEN - 1] = '\0';
	proto->lv = atoi(row[2]);
	memcpy(proto->data, row[3], lengths[3]);
	proto->data_size = lengths[3];
	proto->player_id = req->player_id;
	proto->target_id = player_id;

	free_query(res);

	LOG_DEBUG("[%s: %d]: get user info: player: %lu[%lu], comm_data size: %u", __FUNCTION__, __LINE__, req->player_id, req->target_id, proto->data_size);
	fast_send_msg_base(server_node, extern_data, SERVER_PROTO_DOUFACHANG_LOAD_PLAYER_ANSWER, sizeof(*proto) + proto->data_size, 0);
}

void conn_node_dbsrv::handle_enter_game(EXTERN_DATA *extern_data)
{
	PROTO_ENTER_GAME_REQ* request = (PROTO_ENTER_GAME_REQ *)conn_node_base::buf_head();
	uint64_t player_id = request->player_id;

	unsigned long *lengths;	
	MYSQL_RES *res = NULL;
	MYSQL_ROW row;

	int ret=0;

	LOG_INFO("[%s : %d]: openid[%u] player [%lu]", __FUNCTION__, __LINE__, extern_data->open_id, player_id);

	query(const_cast<char*>("set names utf8"), 1, NULL);

	if (extern_data->open_id > 0) {
		sprintf(sql, "SELECT job, player_name, lv , comm_data, UNIX_TIMESTAMP(create_time), UNIX_TIMESTAMP(logout_time),chengjie_rest from player where open_id = %u and player_id = %lu", extern_data->open_id, player_id);
	} else {
		LOG_ERR("[%s : %d]: notice: no openid, maybe a bug. openid[%u] player [%lu]", __FUNCTION__, __LINE__, extern_data->open_id, player_id);
		sprintf(sql, "SELECT job, player_name, lv , comm_data, UNIX_TIMESTAMP(create_time), UNIX_TIMESTAMP(logout_time),chengjie_rest from player where player_id = %lu", player_id);
	}

	res = query(sql, 1, NULL);
	if (!res) {
		LOG_ERR("[%s : %d]: query user failed, sql: %s", __FUNCTION__, __LINE__, sql);
		return;
	}
	row = fetch_row(res);
	if (!row) {
		LOG_ERR("[%s : %d]: query sql fetch row failed, sql: %s", __FUNCTION__, __LINE__, sql);
		free_query(res);
		return;
	}

	lengths = mysql_fetch_lengths(res);

	PROTO_ENTER_GAME_RESP *proto = (PROTO_ENTER_GAME_RESP *)&conn_node_dbsrv::global_send_buf[0];
	proto->reconnect = request->reconnect;
	proto->job = atoi(row[0]) & 0xff;
	strncpy(proto->name, row[1], MAX_PLAYER_NAME_LEN);
	proto->name[MAX_PLAYER_NAME_LEN - 1] = '\0';
	proto->lv = atoi(row[2]);
	proto->plug = 0;
	memcpy(proto->data, row[3], lengths[3]);
	proto->data_size = lengths[3];
	proto->create_time = (row[4] != NULL ? atoi(row[4]) : 0);
	proto->logout_time = (row[5] != NULL ? atoi(row[5]) : 0);
	proto->chengjie_cd = atol(row[6]);

	proto->head.msg_id = ENDION_FUNC_2(SERVER_PROTO_ENTER_GAME_ANSWER);
	proto->head.len = ENDION_FUNC_4(sizeof(PROTO_ENTER_GAME_RESP) + proto->data_size);
	proto->player_id = player_id;
	//proto->platform = request->platform;
	//proto->ad_channel = request->ad_channel;

	//memcpy(proto->open_id, request->open_id, sizeof(request->open_id));
	//memcpy(proto->channel, request->channel, sizeof(request->channel));

	//proto->open_id[49] = '\0';
	//proto->channel[49]= '\0';
	free_query(res);

	proto->guild_id = 0;
	do
	{
		sprintf(sql, "SELECT `guild_id` from guild_player where player_id = %lu", player_id);
		res = query(sql, 1, NULL);
		if (!res) {
			LOG_ERR("[%s : %d]: query user failed, sql: %s", __FUNCTION__, __LINE__, sql);
			break;
		}
		row = fetch_row(res);
		if (!row) {
			LOG_ERR("[%s : %d]: query sql fetch row failed, sql: %s", __FUNCTION__, __LINE__, sql);
			free_query(res);
			break;
		}

		proto->guild_id = atoi(row[0]);

		free_query(res);
	} while(0);

	if (extern_data->player_id == 0)
		extern_data->player_id = player_id;

	LOG_DEBUG("[%s: %d]: get user info: player: %lu, lv: %u, comm_data size: %u", __FUNCTION__, __LINE__, extern_data->player_id, proto->lv, proto->data_size);
	add_extern_data(&proto->head, extern_data);

	ret = server_node->send_one_msg(&proto->head, 1);
	if (ret != (int)ENDION_FUNC_4(proto->head.len)) {
		LOG_ERR("[%s:%d] send to game srv failed err[%d], ret: %d, len: %u", __FUNCTION__, __LINE__, errno, ret, proto->head.len);
	} else {
		ret = 0;
		handle_load_client_data(extern_data);
	}
	
}

void conn_node_dbsrv::handle_rename_request(EXTERN_DATA *extern_data)
{
	int ret = 0;
	unsigned long effect = 0;
	std::string szName;
	char sql[1024];
	char* p = NULL;
	int len = 0;

	PlayerRenameRequest* req = player_rename_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req) {
		LOG_ERR("[%s : %d]: unpack rename pack failed", __FUNCTION__, __LINE__);
		ret = ERROR_ID_NAME_EXIST;
		goto done;
	}

	szName = std::string(req->name);
	
	len = snprintf(sql, sizeof(sql), "update player set player_name=\'");
	p = sql + len;
	p += escape_string(p, (const char *)req->name, strlen(req->name));
	sprintf(p, "\' where player_id=%.0f;", (double)extern_data->player_id);

	player_rename_request__free_unpacked(req, NULL);

	query(const_cast<char*>("set names utf8"), 1, NULL);
	query(sql, 1, &effect);

	if (effect == 0) {
		ret = ERROR_ID_NAME_EXIST;
	}
	
done:
	PLAYER_RENAME_DB_ANSWER *resp = (PLAYER_RENAME_DB_ANSWER *)get_send_buf(SERVER_PROTO_RENAME_ANSWER, get_seq());
	resp->head.len = htons(sizeof(PLAYER_RENAME_DB_ANSWER));
	resp->result = ret;
	strcpy(resp->name, szName.c_str());
	add_extern_data(&resp->head, extern_data);
	if (!server_node || server_node->send_one_msg(&resp->head, 1) != (int32_t)ENDION_FUNC_4(resp->head.len)) {
		LOG_ERR("%s %d: send to gamesrv failed err[%d]", __FUNCTION__, __LINE__, errno);						
	}
}

void conn_node_dbsrv::handle_save_client_data(EXTERN_DATA *extern_data)
{
	int ret = 0;
	do
	{
		uint64_t effect = 0;
		int len;
		char *p;

		len = sprintf(sql, "replace client_data set player_id = %lu, data = \'", extern_data->player_id);
		p = sql + len;
		p += escape_string(p, (const char*)get_data(), get_data_len());
		len = sprintf(p, "\'");

		LOG_DEBUG("[%s:%d] playerid: %lu, data size: %d.", __FUNCTION__, __LINE__, extern_data->player_id, get_data_len());
		query(sql, 1, &effect);	
		if (effect <= 0) 
		{
			ret = ERROR_ID_MYSQL_QUERY;
			LOG_ERR("[%s:%d] save %lu failed effect[%lu]", __FUNCTION__, __LINE__, extern_data->player_id, effect);
			break;
		}
	} while(0);

	CommAnswer resp;
	comm_answer__init(&resp);

	resp.result = ret;
	fast_send_msg(server_node, extern_data, MSG_ID_SAVE_CLIENT_DATA_ANSWER, comm_answer__pack, resp);
}

void conn_node_dbsrv::handle_load_client_data(EXTERN_DATA *extern_data)
{
	uint64_t player_id = extern_data->player_id;
	unsigned long *lengths = NULL;	
	MYSQL_RES *res = NULL;
	MYSQL_ROW row;
	size_t data_size = 0;

	LOG_INFO("[%s : %d]: openid[%u] player [%lu]", __FUNCTION__, __LINE__, extern_data->open_id, player_id);

	do
	{
//		query(const_cast<char*>("set names utf8"), 1, NULL);

		sprintf(sql, "SELECT data from client_data where player_id = %lu", player_id);

		res = query(sql, 1, NULL);
		if (!res) {
			LOG_ERR("[%s : %d]: query user failed, sql: %s", __FUNCTION__, __LINE__, sql);
			break;
		}
		row = fetch_row(res);
		if (!row) {
			LOG_ERR("[%s : %d]: query sql fetch row failed, sql: %s", __FUNCTION__, __LINE__, sql);
//			free_query(res);
			break;
		}

		lengths = mysql_fetch_lengths(res);
		data_size = lengths[0];
	} while(0);

	// LoadClientDataAnswer resp;
	// load_client_data_answer__init(&resp);

	// resp.result = 0;
	// if (data_size > 0)
	// {
	// 	resp.data = row[0];
	// }

	// fast_send_msg(server_node, extern_data, MSG_ID_LOAD_CLIENT_DATA_ANSWER, load_client_data_answer__pack, resp);

	PROTO_HEAD* head = get_send_buf(MSG_ID_LOAD_CLIENT_DATA_ANSWER, get_seq());
	if (data_size > 0)
	{
		memcpy(head->data, row[0], data_size);
	}
	head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD) + data_size);
	server_node->add_extern_data(head, extern_data);
//	ret = fast_send_msg_base(server_node, extern_data, msg_id, size, seq);
	int ret = server_node->send_one_msg(head, 1);
	if (ret != (int)ENDION_FUNC_4(head->len)) {
		LOG_ERR("[%s:%d] send to game srv failed err[%d], ret: %d, len: %u", __FUNCTION__, __LINE__, errno, ret, head->len);
	} else {
		ret = 0;
	}

	if (res)
		free_query(res); 
}

void conn_node_dbsrv::handle_search_player(EXTERN_DATA *extern_data)
{
	FriendSearchRequest *req = friend_search_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return ;
	}

	unsigned long *lengths;	
	MYSQL_RES *res = NULL;
	MYSQL_ROW row;

	LOG_INFO("[%s:%d] player[%lu], search_id[%lu], search_name[%s]", __FUNCTION__, __LINE__, extern_data->player_id, req->playerid, req->playername);

	query(const_cast<char*>("set names utf8"), 1, NULL);
	if (req->playerid > 0)
	{
		sprintf(sql, "SELECT job, player_name, lv , comm_data, UNIX_TIMESTAMP(create_time), UNIX_TIMESTAMP(logout_time),chengjie_rest, player_id from player where player_id = %lu", req->playerid);
	}
	else
	{
		char *p = sql;
		p += sprintf(p, "SELECT job, player_name, lv , comm_data, UNIX_TIMESTAMP(create_time), UNIX_TIMESTAMP(logout_time),chengjie_rest, player_id from player where player_name = \'");
		p += escape_string(p, req->playername, strlen(req->playername));
		p += sprintf(p, "\'");
	}

	PROTO_ENTER_GAME_RESP *proto = (PROTO_ENTER_GAME_RESP *)&conn_node_dbsrv::global_send_buf[0];
	memset(proto, 0, sizeof(PROTO_ENTER_GAME_RESP));
	do
	{
		res = query(sql, 1, NULL);
		if (!res) {
			LOG_ERR("[%s : %d]: query user failed, sql: %s", __FUNCTION__, __LINE__, sql);
			break;
		}
		row = fetch_row(res);
		if (!row) {
			LOG_ERR("[%s : %d]: query sql fetch row failed, sql: %s", __FUNCTION__, __LINE__, sql);
			break;
		}

		lengths = mysql_fetch_lengths(res);

		proto->job = atoi(row[0]) & 0xff;
		strncpy(proto->name, row[1], MAX_PLAYER_NAME_LEN);
		proto->name[MAX_PLAYER_NAME_LEN - 1] = '\0';
		proto->lv = atoi(row[2]);
		proto->plug = 0;
		memcpy(proto->data, row[3], lengths[3]);
		proto->data_size = lengths[3];
		proto->create_time = (row[4] != NULL ? atoi(row[4]) : 0);
		proto->logout_time = (row[5] != NULL ? atoi(row[5]) : 0);
		proto->chengjie_cd = atol(row[6]);
		proto->player_id = atol(row[7]);

		//proto->platform = request->platform;
		//proto->ad_channel = request->ad_channel;

		//memcpy(proto->open_id, request->open_id, sizeof(request->open_id));
		//memcpy(proto->channel, request->channel, sizeof(request->channel));

		//proto->open_id[49] = '\0';
		//proto->channel[49]= '\0';

	} while(0);

	if (res)
	{
		free_query(res);
		res = NULL;
	}

	proto->head.msg_id = ENDION_FUNC_2(MSG_ID_FRIEND_SEARCH_ANSWER);
	proto->head.len = ENDION_FUNC_4(sizeof(PROTO_ENTER_GAME_RESP) + proto->data_size);

	add_extern_data(&proto->head, extern_data);

	int ret = server_node->send_one_msg(&proto->head, 1);
	if (ret != (int)ENDION_FUNC_4(proto->head.len))
	{
		LOG_ERR("[%s:%d] send to game srv failed err[%d], ret: %d, len: %d", __FUNCTION__, __LINE__, errno, ret, (int)ENDION_FUNC_4(proto->head.len));
	}
	else
	{
		ret = 0;
	}
	
	friend_search_request__free_unpacked(req, NULL);
}

void conn_node_dbsrv::handle_get_other_info(EXTERN_DATA *extern_data)
{
	GetOtherInfoRequest *req = get_other_info_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return ;
	}
	
	uint64_t target_id = req->playerid;

	get_other_info_request__free_unpacked(req, NULL);

	unsigned long *lengths;	
	MYSQL_RES *res = NULL;
	MYSQL_ROW row;

	LOG_INFO("[%s:%d] player[%lu], target_id[%lu]", __FUNCTION__, __LINE__, extern_data->player_id, target_id);

	query(const_cast<char*>("set names utf8"), 1, NULL);
	sprintf(sql, "SELECT job, player_name, lv , comm_data, UNIX_TIMESTAMP(create_time), UNIX_TIMESTAMP(logout_time),chengjie_rest, player_id from player where player_id = %lu", target_id);

	PROTO_ENTER_GAME_RESP *proto = (PROTO_ENTER_GAME_RESP *)&conn_node_dbsrv::global_send_buf[0];
	memset(proto, 0, sizeof(PROTO_ENTER_GAME_RESP));
	do
	{
		res = query(sql, 1, NULL);
		if (!res) {
			LOG_ERR("[%s : %d]: query user failed, sql: %s", __FUNCTION__, __LINE__, sql);
			break;
		}
		row = fetch_row(res);
		if (!row) {
			LOG_ERR("[%s : %d]: query sql fetch row failed, sql: %s", __FUNCTION__, __LINE__, sql);
			break;
		}

		lengths = mysql_fetch_lengths(res);

		proto->job = atoi(row[0]) & 0xff;
		strncpy(proto->name, row[1], MAX_PLAYER_NAME_LEN);
		proto->name[MAX_PLAYER_NAME_LEN - 1] = '\0';
		proto->lv = atoi(row[2]);
		proto->plug = 0;
		memcpy(proto->data, row[3], lengths[3]);
		proto->data_size = lengths[3];
		proto->create_time = (row[4] != NULL ? atoi(row[4]) : 0);
		proto->logout_time = (row[5] != NULL ? atoi(row[5]) : 0);
		proto->chengjie_cd = atol(row[6]);
		proto->player_id = atol(row[7]);

		//proto->platform = request->platform;
		//proto->ad_channel = request->ad_channel;

		//memcpy(proto->open_id, request->open_id, sizeof(request->open_id));
		//memcpy(proto->channel, request->channel, sizeof(request->channel));

		//proto->open_id[49] = '\0';
		//proto->channel[49]= '\0';

	} while(0);

	if (res)
	{
		free_query(res);
		res = NULL;
	}

	proto->head.msg_id = ENDION_FUNC_2(MSG_ID_GET_OTHER_INFO_ANSWER);
	proto->head.len = ENDION_FUNC_4(sizeof(PROTO_ENTER_GAME_RESP) + proto->data_size);

	add_extern_data(&proto->head, extern_data);

	int ret = server_node->send_one_msg(&proto->head, 1);
	if (ret != (int)ENDION_FUNC_4(proto->head.len))
	{
		LOG_ERR("[%s:%d] send to game srv failed err[%d], ret: %d, len: %d", __FUNCTION__, __LINE__, errno, ret, (int)ENDION_FUNC_4(proto->head.len));
	}
	else
	{
		ret = 0;
	}
}

void conn_node_dbsrv::handle_load_server_level(EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d]", __FUNCTION__, __LINE__); 

	unsigned long *lengths = NULL;	
	MYSQL_RES *res = NULL;
	MYSQL_ROW row = NULL;

	size_t data_size = 0;

	query(const_cast<char*>("set names utf8"), 1, NULL);

	sprintf(sql, "SELECT comm_data from game_global where key = %u", server_level_key);
	res = query(sql, 1, NULL);
	if (res)
	{
		row = fetch_row(res);
		if (row)
		{
			lengths = mysql_fetch_lengths(res);
			data_size = lengths[0];
			memcpy(get_send_data(), row[0], data_size);
		}
		free_query(res);
	}

	fast_send_msg_base(server_node, extern_data, SERVER_PROTO_LOAD_SERVER_LEVEL_ANSWER, data_size, 0);
}

void conn_node_dbsrv::handle_save_server_level(EXTERN_DATA *extern_data)
{
	uint64_t effect = 0;
	int len;
	char *p;

	len = sprintf(sql, "replace game_global set key = %u, data = \'", server_level_key);
	p = sql + len;
	p += escape_string(p, (const char*)get_data(), get_data_len());
	len = sprintf(p, "\'");

	LOG_DEBUG("[%s:%d] data size: %d.", __FUNCTION__, __LINE__, get_data_len());
	query(sql, 1, &effect);	
	if (effect <= 0) 
	{
		LOG_ERR("[%s:%d] save failed effect[%lu]", __FUNCTION__, __LINE__, effect);
		return ;
	}
}

void conn_node_dbsrv::handle_break_server_level(EXTERN_DATA *extern_data)
{
	LOG_INFO("[%s:%d]", __FUNCTION__, __LINE__); 

	uint32_t *pData = (uint32_t*)get_data();
	uint32_t level_limit = *pData++;

	MYSQL_RES *res = NULL;
	MYSQL_ROW row = NULL;

	int num = 0;

	sprintf(sql, "SELECT count(player_id) from player where lv >= %u", level_limit);
	res = query(sql, 1, NULL);
	if (res)
	{
		row = fetch_row(res);
		if (row)
		{
			num = atoi(row[0]);
		}
		free_query(res);
	}

	uint32_t *send_data = (uint32_t*)get_send_data();
	*send_data++ = num;
	fast_send_msg_base(server_node, extern_data, SERVER_PROTO_BREAK_SERVER_LEVEL_ANSWER, sizeof(uint32_t), 0);
}

//////////////////////////// 涓嬮潰鏄痵tatic 鍑芥暟
conn_node_dbsrv *conn_node_dbsrv::server_node;
