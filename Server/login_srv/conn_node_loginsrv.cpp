#include "conn_node_loginsrv.h"
#include "game_event.h"
#include "time_helper.h"
#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <sstream>
#include <string.h>
#include <map>
#include <unistd.h>
#include "mysql_module.h"
#include "server_proto.h"
#include <openssl/md5.h>
#include <curl/curl.h>
#include <vector>
//#include "app_data_statis.h"
//#include "ChatType.h"
#include "stl_relation.h"
#include "msgid.h"
#include "error_code.h"
#include "attr_id.h"
#include "cgi_common.h"
#include "comm_message.pb-c.h"
#include "login.pb-c.h"
#include "player_db.pb-c.h"
#include "login_config.h"
#include "game_helper.h"

/**每个帐号在每个服最多10个角色**/
#define MAX_PLAYER_NUM  10
/**名字的最大长度**/
//#define MAX_PLAYER_NAME_LEN  255

conn_node_loginsrv conn_node_loginsrv::connecter;
static char sql[1024*64];
//static char send_buf[1024 * 64];
static PlayerBaseInfo playerinfo_buf[MAX_PLAYER_NUM];
static PlayerBaseInfo *playerinfo[MAX_PLAYER_NUM];
static char playerinfo_name[MAX_PLAYER_NUM][MAX_PLAYER_NAME_LEN];

uint32_t sg_server_id = 0;
uint32_t sg_max_conn = 0;
std::string sg_server_key;
std::map<uint32_t, std::pair<uint32_t, uint32_t> > sg_filter_list;
long sg_filter_map_locker;

struct event sg_event_timer = {0};
struct timeval sg_timeout = {300, 0};	

// std::set<uint32_t>  filterPlayerList;
static const char* scg_appkey = "PzN4xoVvh3QOg79yfibUKmDHcswRIu8p";


#define PROTO_STATIS_INFO_COMMIT3(magicId, num1, ullPlayerid)     \
	do {  \
	PROTO_STATIS_INFO* proto = (PROTO_STATIS_INFO*)get_send_buf(SERVER_PROTO_STATIS_INFO, 0);  \
	proto_statis_info__init(proto, COMMIT_MYSQL, GAME_APPID, extern_data->open_id, ullPlayerid, sg_server_id, (uint32_t)time(NULL)  \
	, magicId, num1, 0, 0, 0, 0);  \
	proto->head.len = htons(sizeof(PROTO_STATIS_INFO)); \
	add_extern_data((PROTO_HEAD*)proto, extern_data);  \
	int result  = connecter.send_one_msg((PROTO_HEAD*)proto, 1); \
	if (result != htons(proto->head.len)) {  \
	LOG_ERR("%s: send to client failed err[%d]", __FUNCTION__, errno); \
	}  \
	} while(false)  \


conn_node_loginsrv::conn_node_loginsrv()
{
	max_buf_len = 1024 * 1024;
	buf = (uint8_t *)malloc(max_buf_len + sizeof(EXTERN_DATA));
	assert(buf);
	sg_max_conn = 1000;
}

conn_node_loginsrv::~conn_node_loginsrv()
{
}

int conn_node_loginsrv::handle_list_player(EXTERN_DATA *extern_data)
{
	PlayerListAnswer resp;
	player_list_answer__init(&resp);
	resp.result = ERROR_ID_SUCCESS;

	PROTO_HEAD *head = get_send_buf(MSG_ID_PLAYER_LIST_ANSWER, get_seq());

	select_player_base_info(extern_data->open_id, &resp.n_playerlist);
	resp.playerlist = playerinfo;
	
	size_t size = player_list_answer__pack(&resp, (uint8_t *)head->data);
	head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD) + size);
	add_extern_data(head, extern_data);

	int ret = conn_node_loginsrv::connecter.send_one_msg(head, 1);
	if (ret != (int)ENDION_FUNC_4(head->len)) {
		LOG_ERR("%s: send to client failed err[%d]", __FUNCTION__, errno);
		return (0);
	}
	
	return (0);
}

int conn_node_loginsrv::check_user_exist_map(uint32_t openid)
{
	uint64_t effect = 0;
	char buffer[1024];
	MYSQL_RES *res = NULL;
	MYSQL_ROW row = NULL;	

	if (openid == 0 ) {
		LOG_ERR("[%s : %d]: input param is null", __FUNCTION__, __LINE__);
		return -1;
	}

	snprintf(buffer, sizeof(buffer), "select count(open_id) as total from player where open_id=%u", openid);	

	query(const_cast<char*>("set names utf8"), 1, NULL);
	res = query(buffer, 1, &effect);
	if (!res) {
		LOG_ERR("[%s: %d]: query user map failed, sql: %s", __FUNCTION__, __LINE__, buffer);
		return -1;
	}

	row = fetch_row(res);
	if (!row) {
		LOG_ERR("[%s: %d]: fetch rwo failed, sql: %s", __FUNCTION__, __LINE__, buffer);
		free_query(res);
		return -1;
	}

	uint32_t total = strtoul(row[0], NULL, 0);
	if (total<=0) {
		free_query(res);
		return -1;
	}	

	free_query(res);

	return 0;
}

int conn_node_loginsrv::handle_login(EXTERN_DATA *extern_data)
{
	int ret = 0;
	uint32_t nChannel = 0;
	std::string openid ;
	std::string key;
	uint32_t nOpenId = 0;
	uint16_t seq = get_seq();

	std::ostringstream os;
	std::string szOut;

	std::map<uint32_t, std::pair<uint32_t, uint32_t> >::iterator iter;

	LoginRequest *req;
	req = login_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req) {
		ret = -1;
		goto done;
	}

	nChannel = req->channel;
	openid = req->openid;
	key = req->key;
	
	login_request__free_unpacked(req, NULL);

	nOpenId = strtoul(openid.c_str(), NULL, 0);
// 	if (filterPlayerList.find(nOpenId) != filterPlayerList.end()) {
// 		LOG_ERR("[%s : %d]: open id in filter list, openid: %u", __FUNCTION__, __LINE__, nOpenId);
// 		ret = ERRORCODE_PLAYER_FENG_HAO;
// 		goto done;
// 	}

	if (nOpenId == 0) {
		LOG_ERR("[%s : %d]: invalid open id, open_id: %s, key: %s, nOpenId: %u", __FUNCTION__, __LINE__, openid.c_str(), key.c_str(), nOpenId);
		ret = -1;
		goto done;
	}

	iter = sg_filter_list.find(nOpenId);
	if (iter != sg_filter_list.end()) {
		uint32_t now = time(NULL);
		if (iter->second.first == 0 || (now-iter->second.second)/86400 < iter->second.first) {
			LOG_ERR("[%s : %d]: filter openid: %u, day: %u, ts: %u, now: %u", __FUNCTION__, __LINE__, nOpenId, iter->second.first, iter->second.second, now);
			ret = ERROR_ID_ACCOUNT_BANNED;
			goto done;
		}
	}

	//ret = check_user_exist_map(nOpenId);
	//if (0 != ret) {
	//	ret = check_can_create_player();
	//	if (0!=ret) {
	//		LOG_ERR("[%s : %d]: insert user to map failed, open_id: %s player full", __FUNCTION__, __LINE__, openid.c_str());
	//		ret = ERRORCODE_SRV_PERSONNEL_FULL;
	//		goto done;
	//	}
	//}
	
	os << "open_id:"<< nOpenId <<",channel:" << nChannel << ",scg_appkey:" << scg_appkey;

	LOG_INFO("[%s: %d]: os %s", __FUNCTION__, __LINE__, os.str().c_str());
	
	szOut = create_md5_val(os.str());

	if (key != szOut) {
		LOG_ERR("[%s : %d]: invalid key, open_id: %s, key: %s, nOpenId: %u", __FUNCTION__, __LINE__, openid.c_str(), key.c_str(), nOpenId);
		LOG_ERR("[%s : %d]: invalid key, out1[%s], nOpenId: %u", __FUNCTION__, __LINE__, szOut.c_str(), nOpenId);		
		ret = ERROR_ID_LOGIN_TOKEN;
		goto done;
	}

done:
	PROTO_ROLE_LOGIN* proto = (PROTO_ROLE_LOGIN*)get_send_buf(SERVER_PROTO_LOGIN, seq);
	proto->head.len = ENDION_FUNC_4(sizeof(proto));
	//proto.head.msg_id = ENDION_FUNC_2(SERVER_PROTO_LOGIN);
	//proto.head.seq = 0;

	extern_data->player_id = 0;
	extern_data->open_id = nOpenId;
	add_extern_data(&proto->head, extern_data);
		//todo check passwd, get openid
	proto->result = ret;
	proto->login_seq = seq;

	ret = conn_node_loginsrv::connecter.send_one_msg(&proto->head, 1);
	if (ret != (int)ENDION_FUNC_4(proto->head.len)) {
		LOG_ERR("%s: send to conn srv failed err[%d]", __FUNCTION__, errno);
	}
	
	return (0);
}

int conn_node_loginsrv::handle_delete_player(EXTERN_DATA *extern_data)
{
/*	uint64_t player_id;
	uint32_t result = ERRORCODE_RESULT_ID_SUCCESS;
	if (extern_data->open_id == 0)
		return (-1);
	
	DeletePlayerRequest *req;
	req = delete_player_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req) {
		return (-10);
	}
	player_id = req->playerid;
	delete_player_request__free_unpacked(req, NULL);
	
	LOG_DEBUG("%s: delete_player [%lu][%lu]", __FUNCTION__, extern_data->open_id, player_id);
	
	uint64_t effect = 0;
	sprintf(sql, "DELETE  player where open_id = %u and player_id = %lu", extern_data->open_id, player_id);
	query(sql, 1, &effect);	
	if (effect != 1) 
	{
		result = RESULT_ID_FAIL;
	}
	

	DeletePlayerAnswer resp;	
	delete_player_answer__init(&resp);
	resp.result = result;
	resp.playerid = player_id;
	
	PROTO_HEAD *head = get_send_buf(DELETE_PLAYER_ANSWER, 0);
	
	int ret = conn_node_loginsrv::connecter.send_one_msg(head, 1);
	if (ret != htons(head->len)) {
		LOG_ERR("%s: send to client failed err[%d]", __FUNCTION__, errno);
		return (0);
	}
*/	return (0);
}

static std::vector<uint32_t> attrs_id[MAX_PLAYER_NUM];
static std::vector<uint32_t> attrs_val[MAX_PLAYER_NUM];

int conn_node_loginsrv::select_player_base_info(uint32_t open_id, size_t *n_playerinfo, uint64_t player_id)
{
	MYSQL_RES *res = NULL;
	MYSQL_ROW row;
	
	*n_playerinfo = 0;

	for (int i=0; i<MAX_PLAYER_NUM; ++i) {
		attrs_id[i].clear();
		attrs_val[i].clear();
	}
	

	query(const_cast<char*>("set names utf8"), 1, NULL);
	if (player_id == 0)
	{
		sprintf(sql, "SELECT player_id, job, player_name, lv, comm_data from player where open_id = %u", open_id);
	}
	else
	{
		sprintf(sql, "SELECT player_id, job, player_name, lv, comm_data from player where open_id = %u and player_id = %lu", open_id, player_id);
	}
	
	res = query(sql, 1, NULL);
	if (!res) {
			//todo
		return (-1);
	}

	for (;;) {
		if (*n_playerinfo >= MAX_PLAYER_NUM)
			break;
		row = fetch_row(res);
		if (!row)
			break;

		
		unsigned long* lengths = mysql_fetch_lengths(res);
		playerinfo[*n_playerinfo] = &playerinfo_buf[*n_playerinfo];
		player_base_info__init(playerinfo[*n_playerinfo]);

		PlayerDBInfo *db_info = player_dbinfo__unpack(NULL, lengths[4], (uint8_t*)row[4]);

		if (!db_info) {
			LOG_ERR("%s %d: unpack dbinfo fail", __FUNCTION__, __LINE__);
			continue;
		}
		
		playerinfo[*n_playerinfo]->name = &playerinfo_name[*n_playerinfo][0];
		playerinfo[*n_playerinfo]->playerid = strtoull(row[0], NULL, 10);

		uint32_t job = atoi(row[1]);

		attrs_id[*n_playerinfo].push_back(PLAYER_ATTR_JOB);
		attrs_val[*n_playerinfo].push_back(job);

		strncpy(playerinfo[*n_playerinfo]->name, row[2], MAX_PLAYER_NAME_LEN - 1);
		playerinfo[*n_playerinfo]->name[MAX_PLAYER_NAME_LEN - 1] = '\0';		
		playerinfo[*n_playerinfo]->name = strdup(row[2]);


		attrs_id[*n_playerinfo].push_back(PLAYER_ATTR_LEVEL);
		attrs_val[*n_playerinfo].push_back(atoi(row[3]));

		for (size_t i = 0; i < db_info->n_attr_id; ++i)
		{
			attrs_id[*n_playerinfo].push_back(db_info->attr_id[i]);
			attrs_val[*n_playerinfo].push_back(db_info->attr[i]);
		}

		playerinfo[*n_playerinfo]->n_attrid = playerinfo[*n_playerinfo]->n_attrval  = attrs_id[*n_playerinfo].size();
		if (attrs_id[*n_playerinfo].size() > 0) {
			playerinfo[*n_playerinfo]->attrid = &attrs_id[*n_playerinfo][0];
			playerinfo[*n_playerinfo]->attrval = &attrs_val[*n_playerinfo][0];
		}

		*n_playerinfo += 1;


		player_dbinfo__free_unpacked(db_info, NULL);
		/*
		LOG_INFO("[%s : %d]: PlayerInfo; open id: %u, player id: %lu, level: %d, name: %s, job: %d"
			, __FUNCTION__, __LINE__, open_id, playerinfo[*n_playerinfo]->playerid
			, playerinfo[*n_playerinfo]->level, playerinfo[*n_playerinfo]->name, playerinfo[*n_playerinfo]->job);
			*/
	}
	playerinfo[*n_playerinfo] = NULL;
	free_query(res);

	return (0);
}

int conn_node_loginsrv::handle_enter_game(EXTERN_DATA *extern_data)
{
	EnterGameRequest *req = enter_game_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req) {
		LOG_ERR("[%s : %d]: enter game unpack failed, playerid: %lu", __FUNCTION__, __LINE__, extern_data->player_id);
		return -1;
	}

	uint64_t player_id = req->playerid;
	uint32_t reconnect = req->reconnect;
	enter_game_request__free_unpacked(req, NULL);

	MYSQL_RES *res = NULL;
	MYSQL_ROW row;

	snprintf(sql, sizeof(sql), "select count(1) as total from player where open_id=%u and player_id=%lu", extern_data->open_id, player_id);
	
	res = query(sql, 1, NULL);
	if (!res) {
		LOG_ERR("[%s : %d]: query user failed, sql: %s", __FUNCTION__, __LINE__, sql);
		return -1;
	}

	row = fetch_row(res);
	if (!row) {
		LOG_ERR("[%s : %d]: query sql fetch row failed, sql: %s", __FUNCTION__, __LINE__, sql);
		free_query(res);
		return -1;
	}

	uint32_t total = strtoul(row[0], NULL, 0);
	if (total != 1) {
		LOG_ERR("[%s : %d]: player info failed, sql: %s", __FUNCTION__, __LINE__, sql);
		free_query(res);
		return -1;
	}
	free_query(res);

	EXTERN_DATA  ext_data;
	PROTO_ENTER_GAME_REQ* request = (PROTO_ENTER_GAME_REQ *)conn_node_base::get_send_buf(SERVER_PROTO_ENTER_GAME_REQUEST, 0);

	ext_data.player_id = player_id;
	ext_data.open_id = extern_data->open_id;
	ext_data.fd = extern_data->fd;

	request->player_id = player_id;
	request->reconnect = reconnect;

	request->head.len = ENDION_FUNC_4(sizeof(PROTO_ENTER_GAME_REQ));
	request->head.seq = 0;

	conn_node_loginsrv::add_extern_data(&request->head, &ext_data);

	int ret = conn_node_loginsrv::connecter.send_one_msg(&request->head, 1);
	if (ret != (int)ENDION_FUNC_4(request->head.len)) {
		LOG_ERR("[%s:%d] send to conn srv failed err[%d]", __FUNCTION__, __LINE__, errno);
	} else {
		ret = 0;
	}


	return 0;
}

int conn_node_loginsrv::handle_get_player_info_request(EXTERN_DATA *extern_data)
{
/*	unsigned long *lengths;	
	MYSQL_RES *res = NULL;
	MYSQL_ROW row;

	int ret;

	do 
	{
		PROTO_GET_PLAYER_INFO_REQUEST *head = (PROTO_GET_PLAYER_INFO_REQUEST *)buf_head();
		uint64_t player_id = head->playerid;	

		LOG_INFO("[%s : %d]: start, self playerid: %lu， find playerid: %lu", __FUNCTION__, __LINE__, extern_data->player_id, player_id);
		query(const_cast<char*>("set names utf8"), 1, NULL);
		sprintf(sql, "SELECT job, player_name, lv , comm_data, comm_data2 from player where player_id = %lu", player_id);
		res = query(sql, 1, NULL);
		if (!res) {
			//todo
			break;
		}
		row = fetch_row(res);
		if (!row)
			break;

		lengths = mysql_fetch_lengths(res);

		PROTO_GET_PLAYER_INFO_ANSWER *proto = (PROTO_GET_PLAYER_INFO_ANSWER *)&conn_node_base::global_send_buf[0];

		proto->raidid = head->raidid;		
		proto->player_id = player_id;
		proto->msgid = head->msgid;		
			
		proto->job = atoi(row[0]) & 0xff;
		strncpy(proto->name, row[1], MAX_PLAYER_NAME_LEN);
		proto->name[MAX_PLAYER_NAME_LEN - 1] = '\0';
		proto->lv = atoi(row[2]);
		memcpy(proto->data, row[3], lengths[3]);
		proto->data1_size = lengths[3];
		memcpy(proto->data + proto->data1_size, row[4], lengths[4]);
		proto->data2_size = lengths[4];

		proto->head.msg_id = htons(SERVER_PROTO_GET_PLAYER_INFO_ANSWER);
		proto->head.len = htons(0);
		proto->head.crc = htonl(sizeof(PROTO_GET_PLAYER_INFO_ANSWER) + proto->data1_size + proto->data2_size);

		proto->machai_id = head->machai_id;
		proto->target_machai_id = head->target_machai_id;

		add_extern_data(&proto->head, extern_data);

		ret = conn_node_loginsrv::connecter.send_one_msg(&proto->head, 1);
		if (ret != (int)htonl(proto->head.crc)) {
			LOG_ERR("%s: send to conn srv failed err[%d]", __FUNCTION__, errno);
		}
	} while (0);
	
	if (res) {
		free_query(res);
	}
	*/
	return (0);
}

int conn_node_loginsrv::insert_user_to_map(int nChannel, const char* source, const char* szOpenId, uint32_t* uOpenId/*=NULL*/)
{
	uint64_t effect = 0;
	char buffer[1024];
	char key[128] = {0};
	char szSource[128] = {0};
	MYSQL_RES *res = NULL;
	MYSQL_ROW row = NULL;	

	if (!szOpenId) {
		LOG_ERR("[%s : %d]: input param is null", __FUNCTION__, __LINE__);
		return -1;
	}

	snprintf(key, sizeof(key), "%s", szOpenId);
	snprintf(szSource, sizeof(szSource), "%s", source);

// 	int len = snprintf(buffer, sizeof(buffer), "insert into user_map (`user_key`, `source`) values(\'");
// 
// 	char* p = buffer + len;
// 	p += escape_string(p, key,  strlen(key));
// 	sprintf(p, "\')");

	int len = snprintf(buffer, sizeof(buffer), "insert into user_map set `user_key` = \'");
	char* p = buffer + len;
	p += escape_string(p, key,  strlen(key));

	len = sprintf(p, "\', `source` = \'");
	p += len;
	p += escape_string(p, szSource,  strlen(szSource));
	sprintf(p, "\'");

	query(const_cast<char*>("set names utf8"), 1, NULL);
	query(buffer, 1, &effect);	

	snprintf(buffer, sizeof(buffer), "SELECT LAST_INSERT_ID()");

	query(const_cast<char*>("set names utf8"), 1, NULL);
	res = query(buffer, 1, &effect);
	if (!res) {
		LOG_ERR("[%s: %d]: query user map failed, sql: %s", __FUNCTION__, __LINE__, buffer);
		return -1;
	}

	row = fetch_row(res);
	if (!row) {
		LOG_ERR("[%s: %d]: fetch rwo failed, sql: %s", __FUNCTION__, __LINE__, buffer);
		free_query(res);
		return -1;
	}

	if (uOpenId) {
		*uOpenId = strtoul(row[0], NULL, 0);
	}	

	free_query(res);

	return 0;
}

int conn_node_loginsrv::get_user_info(EXTERN_DATA *extern_data, uint32_t oped_id, uint64_t player_id, uint32_t msgId, uint32_t& player_lv, uint32_t platform/*=0*/, uint32_t channel/*=0*/)
{
/*	PROTO_ROLE_ENTER *proto = (PROTO_ROLE_ENTER *)&conn_node_base::global_send_buf[0];

	unsigned long *lengths;	
	MYSQL_RES *res = NULL;
	MYSQL_ROW row;

	int ret=0;

	LOG_INFO("%s: openid[%u] player [%lu]", __FUNCTION__, extern_data->open_id, player_id);

	query(const_cast<char*>("set names utf8"), 1, NULL);

	if (oped_id > 0) {
		sprintf(sql, "SELECT job, player_name, lv , comm_data, plug, comm_data2 from player where open_id = %u and player_id = %lu", oped_id, player_id);
	} else {
		sprintf(sql, "SELECT job, player_name, lv , comm_data, plug, comm_data2 from player where player_id = %lu", player_id);
	}
	

	
	res = query(sql, 1, NULL);
	if (!res) {
		//todo
		return (-1);
	}
	row = fetch_row(res);
	if (!row) {
		ret = -1;
		goto done;
	}

	lengths = mysql_fetch_lengths(res);	
	proto->job = atoi(row[0]) & 0xff;
	strncpy(proto->name, row[1], MAX_PLAYER_NAME_LEN);
	proto->name[MAX_PLAYER_NAME_LEN - 1] = '\0';
	proto->lv = atoi(row[2]);
	proto->plug = atoi(row[4]);
	memcpy(proto->data, row[3], lengths[3]);
	proto->data1_size = lengths[3];
	memcpy(proto->data + proto->data1_size, row[5], lengths[5]);
	proto->data2_size = lengths[5];

	proto->head.msg_id = htons(msgId);
	proto->head.len = htons(0);
	proto->head.crc = htonl(sizeof(PROTO_ROLE_ENTER) + proto->data1_size + proto->data2_size);
	proto->player_id = player_id;
	proto->platform = platform;
	proto->ad_channel = channel;
	player_lv = proto->lv;
	

	free_query(res);
//	proto->open_id

	sprintf(sql, "SELECT open_id, user_key  from user_map where open_id = %u", oped_id);

	res = query(sql, 1, NULL);

	if (res && (row = fetch_row(res))) {
		std::string szTmp = row[1];
		size_t pos = szTmp.find('_');
		std::string szOpenID;
		szOpenID.assign(szTmp.c_str()+pos+1);
		strcpy(proto->open_id, szOpenID.c_str());
	} else {
		std::stringstream os;
		os << oped_id;
		uint32_t nLen = os.str().length();
		strncpy(proto->open_id, os.str().c_str(), nLen);
		proto->open_id[nLen]='\0';
	}
	
	
	if (extern_data->player_id == 0)
		extern_data->player_id = player_id;

	LOG_DEBUG("[%s: %d]: get user info: player: %lu, lv: %u, comm_data size: %u", __FUNCTION__, __LINE__, extern_data->player_id, proto->lv, lengths[3]);
	add_extern_data(&proto->head, extern_data);

	ret = conn_node_loginsrv::connecter.send_one_msg(&proto->head, 1);
	if (ret != (int)htonl(proto->head.crc)) {
		LOG_ERR("%s: send to conn srv failed err[%d]", __FUNCTION__, errno);
	} else {
		ret = 0;
	}
done:		
	free_query(res);	
	return (ret);*/
	return 0;
}

//static uint8_t *init_player_db_info;
static size_t init_player_db_info_size;

bool conn_node_loginsrv::check_open_id_exist(uint32_t open_id) {
	MYSQL_ROW row;	
	snprintf(sql, sizeof(sql), "select count(1) as total from player where open_id=%u", open_id);
	MYSQL_RES* res = query(sql, 1, NULL);
	if (!res) {
		//todo
		return false;
	}

	row = fetch_row(res);
	if (!row) {
		free_query(res);
		return false;
	}

	uint32_t ct = atoi(row[0]);
	if (ct > 0) {
		free_query(res);
		return true;
	}

	free_query(res);

	return false;
}

int  conn_node_loginsrv::get_player_count(uint32_t open_id) {
	MYSQL_ROW row;	
	snprintf(sql, sizeof(sql), "select count(player_id) as total from player where open_id=%u", open_id);
	MYSQL_RES* res = query(sql, 1, NULL);
	if (!res) {
		//todo
		return -1;
	}

	row = fetch_row(res);
	if (!row) {
		free_query(res);
		return -1;
	}

	uint32_t ct = atoi(row[0]);

	free_query(res);

	return ct;
}

int conn_node_loginsrv::check_can_create_player(){
	return (0);
	// MYSQL_ROW row;
	// int result = ERROR_ID_SRV_ACCOUNT_FULL;
	// snprintf(sql, sizeof(sql), "select count(distinct(open_id)) as total from player");
	// MYSQL_RES* res = query(sql, 1, NULL);
	// if (!res) {
	// 	return result;
	// }

	// row = fetch_row(res);
	// if (!row) {
	// 	free_query(res);
	// 	return result;
	// }

	// uint32_t num = atoi(row[0]);
	// free_query(res);

	// if (num<sg_max_conn)
	// 	return 0;

	// return result;
};

int conn_node_loginsrv::handle_create_player(EXTERN_DATA *extern_data)
{
	uint32_t result = ERROR_ID_SUCCESS;
	if (extern_data->open_id == 0)
		return (-1);

	uint64_t player_id = 0;
	int ret = 0;
	bool bRet = check_open_id_exist(extern_data->open_id);
	if (!bRet) {
		ret = check_can_create_player();
	} else {
		int count = get_player_count(extern_data->open_id);
		if (count>=5) {
			ret = ERROR_ID_ACCOUNT_PLAYER_FULL;
		}
	}
	
	if (0 == ret) {
		//if (!init_player_db_info) {
		//	PlayerDBInfo info;
		//	player_dbinfo__init(&info);

		//	int init_scene = 10000;
		//	SceneResTable *scene_config = get_config_by_id(init_scene, &scene_res_config);	
		//	if (scene_config)
		//	{
		//		info.pos_x = scene_config->BirthPointX;
		//		info.pos_y = scene_config->BirthPointY;
		//		info.pos_z = scene_config->BirthPointZ;
		//	}
		//	else
		//	{
		//		info.pos_x = 208;
		//		info.pos_z = 42;
		//		info.pos_y = 34.5;
		//	}

		//	info.exp = 0;
		//	info.scene_id = init_scene;

		//	init_player_db_info_size = player_dbinfo__get_packed_size(&info);
		//	//todo
		//	init_player_db_info = (uint8_t *)malloc(init_player_db_info_size);
		//	player_dbinfo__pack(&info, init_player_db_info);
		//}

		PlayerCreateRequest *req;
		req = player_create_request__unpack(NULL, get_data_len(), (uint8_t *)get_data());
		if (!req || !req->name || strlen(req->name)==0 || !(req->job>=JOB_DEFINE_DAO && req->job <=JOB_DEFINE_FAZHANG)) {
			return (-10);
		}
		LOG_INFO("[%s:%d] create_player [%s][%d] result[%u]", __FUNCTION__, __LINE__, req->name, req->job, result);

		{
			PlayerDBInfo info;
			player_dbinfo__init(&info);

			ParameterTable *birth_config = get_config_by_id(PARAM_ID_BIRTH_MAP, &parameter_config);
			if (birth_config && birth_config->n_parameter1 >= 4)
			{
				info.scene_id = birth_config->parameter1[0];
				info.pos_x = birth_config->parameter1[1];
				info.pos_y = birth_config->parameter1[2];
				info.pos_z = birth_config->parameter1[3];
			}

			info.exp = 0;

				//自动补血默认设置
			DBAutoAddHp auto_add_hp;
			dbauto_add_hp__init(&auto_add_hp);
			info.auto_add_hp = &auto_add_hp;
			info.auto_add_hp->auto_add_hp_percent = 50;
			info.auto_add_hp->open_auto_add_hp = 1;
			info.auto_add_hp->auto_add_hp_item_id = 201070049;

			const static int MAX_CREATE_ATTR = 20;
			int arrNum = 0;
			uint32_t arrId[MAX_CREATE_ATTR] = {0};
			uint32_t arrVaual[MAX_CREATE_ATTR] = { 0 };
			uint32_t carrerid = 101000000 + req->job;
			std::map<uint64_t, struct ActorTable *>::iterator it = actor_config.find(carrerid);
			if (it != actor_config.end())
			{
				if (it->second->n_ResId > 0)
				{
					arrId[arrNum] = PLAYER_ATTR_CLOTHES;
					arrVaual[arrNum] = it->second->ResId[0];
					++arrNum;
					std::map<uint64_t, struct ActorFashionTable *>::iterator itFashion = fashion_config.find(it->second->ResId[0]);
					if (itFashion != fashion_config.end())
					{
						if (itFashion->second->n_ColourID1 > 0)
						{
							arrId[arrNum] = PLAYER_ATTR_CLOTHES_COLOR_UP;
							arrVaual[arrNum] = itFashion->second->ColourID1[0];
							++arrNum;
						}
						if (itFashion->second->n_ColourID2 > 0)
						{
							arrId[arrNum] = PLAYER_ATTR_CLOTHES_COLOR_DOWN;
							arrVaual[arrNum] = itFashion->second->ColourID2[0];
							++arrNum;
						}
					}
				}
				if (it->second->n_HairResId > 0)
				{
					arrId[arrNum] = PLAYER_ATTR_HAT;
					arrVaual[arrNum] = it->second->HairResId[0];
					++arrNum;
					std::map<uint64_t, struct ActorFashionTable *>::iterator itFashion = fashion_config.find(it->second->HairResId[0]);
					if (itFashion != fashion_config.end())
					{
						if (itFashion->second->n_ColourID1 > 0)
						{						
							arrId[arrNum] = PLAYER_ATTR_HAT_COLOR;
							arrVaual[arrNum] = itFashion->second->ColourID1[0];
							++arrNum;
						}
					}
				}
				arrId[arrNum] = PLAYER_ATTR_WEAPON;
				arrVaual[arrNum] = it->second->WeaponId;
				++arrNum;
			}

			ParameterTable *resource_config = get_config_by_id(161000252, &parameter_config);
			if (resource_config)
			{
				for (uint32_t i = 0; i + 1 < resource_config->n_parameter1; i = i+2)
				{
					arrId[arrNum] = resource_config->parameter1[i];
					arrVaual[arrNum] = resource_config->parameter1[i+1];
					++arrNum;
				}
			}

			const static int MAX_INIT_BAG = 10;
			DBBagGrid  grid_data[MAX_INIT_BAG];
			DBBagGrid* grid_point[MAX_INIT_BAG];
			info.bag = grid_point;
			info.n_bag = 0;

			ParameterTable *bag_config = get_config_by_id(161000251, &parameter_config);
			if (bag_config)
			{
				for (uint32_t i = 0; i + 1 < bag_config->n_parameter1; i = i+2)
				{
					uint32_t item_id = bag_config->parameter1[i];
					uint32_t item_num = bag_config->parameter1[i+1];
					uint32_t resource_type = 0;
					switch (get_item_type(item_id))
					{
						case ITEM_TYPE_COIN:
							info.coin = item_num;
							break;
						case ITEM_TYPE_ITEM:
							{
								grid_point[info.n_bag] = &grid_data[info.n_bag];
								dbbag_grid__init(&grid_data[info.n_bag]);
								grid_data[info.n_bag].id = item_id;
								grid_data[info.n_bag].num = item_num;
								info.n_bag++;
							}
							break;
					}

					if (resource_type > 0)
					{
						arrId[arrNum] = resource_type;
						arrVaual[arrNum] = item_num;
						++arrNum;
					}
				}
			}

			//伙伴
			arrId[arrNum] = PLAYER_ATTR_PARTNER_FIGHT;
			arrVaual[arrNum] = 1;
			++arrNum;
			arrId[arrNum] = PLAYER_ATTR_PARTNER_PRECEDENCE;
			arrVaual[arrNum] = 1;
			++arrNum;
			
			info.n_attr_id = arrNum;
			info.n_attr = arrNum;
			info.attr_id = arrId;
			info.attr = arrVaual;
			DbHorseCommonAttr sendAttr;
			db_horse_common_attr__init(&sendAttr);
			info.horse_attr = &sendAttr;
			//YaoshiDb yaoshi;
			//yaoshi_db__init(&yaoshi);
			//info.yaoshi = &yaoshi;

			init_player_db_info_size = player_dbinfo__get_packed_size(&info);
			player_dbinfo__pack(&info, conn_node_base::global_send_buf);
		}

		uint64_t effect = 0;
		int len;
		char *p;

		len = sprintf(sql, "INSERT INTO player set open_id = %u, create_time = now(), player_id = %lu, job = %u, player_name = \'",
			extern_data->open_id, extern_data->player_id, req->job);
		p = sql + len;
		p += escape_string(p, req->name, strlen(req->name));
		len = sprintf(p, "\', comm_data = \'");
		p += len;
		p += escape_string(p, (const char *)conn_node_base::global_send_buf, init_player_db_info_size);
		*p++ = '\'';
		*p++ = '\0';

		query(const_cast<char*>("set names utf8"), 1, NULL);
		query(sql, 1, &effect);	
		if (effect != 1) 
		{
			LOG_ERR("[%s:%d] create player fail, open_id = %u, name = %s, job = %u", __FUNCTION__, __LINE__, extern_data->open_id, req->name, req->job);
			result = ERROR_ID_DUPLICATE_NAME;
		}

		MYSQL_RES *res = NULL;
		MYSQL_ROW row = NULL;	
		res = query((char *)("SELECT LAST_INSERT_ID()"), 1, NULL);
		if (res) {
			row = fetch_row(res);
			if (row) {
				player_id = strtoul(row[0], NULL, 0);
			}
			free_query(res);		
		}

		player_create_request__free_unpacked(req, NULL);
	} else {
		result = ret;
	}

	PlayerCreateAnswer resp;
	player_create_answer__init(&resp);
	resp.result = result;

	LOG_DEBUG("[%s:%d] create player return = %u %u, player_id:%lu", __FUNCTION__, __LINE__, resp.result, result, player_id);
	
	PROTO_HEAD *head = get_send_buf(MSG_ID_PLAYER_CREATE_ANSWER, 0);

	size_t player_size = 0;
	select_player_base_info(extern_data->open_id, &player_size, player_id);
	if (player_size == 1)
	{
		resp.playerlist = playerinfo[0];
	}
	
	size_t size = player_create_answer__pack(&resp, (uint8_t *)head->data);
	head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD) + size);
	add_extern_data(head, extern_data);
	
	ret = conn_node_loginsrv::connecter.send_one_msg(head, 1);
	if (ret != (int)ENDION_FUNC_4(head->len)) {
		LOG_ERR("[%s:%d] send to client failed err[%d]", __FUNCTION__, __LINE__, errno);
		return (0);
	}

	if (result == ERROR_ID_SUCCESS) {
//		pack_statis(GAME_APPID, extern_data->open_id, player_id, 0, time(NULL), MAGIC_ID_TYPE_CREATE_PLAYER);
//		PROTO_STATIS_INFO_COMMIT3(MAGIC_ID_TYPE_CREATE_PLAYER, 0, player_id);
	}
	
	return (0);
}

int conn_node_loginsrv::recv_func(evutil_socket_t fd)
{
	EXTERN_DATA *extern_data;
	PROTO_HEAD *head;	
	for (;;) {
		int ret = get_one_buf();
		if (ret == 0) {
			head = (PROTO_HEAD *)buf_head();
			extern_data = get_extern_data(head);
			int cmd = get_cmd();
			switch(cmd)
			{
				case MSG_ID_LOGIN_REQUEST:
				{
					handle_login(extern_data);
					break;
				}
				
				case MSG_ID_PLAYER_LIST_REQUEST:
				{
					handle_list_player(extern_data);
					break;
				}
				case MSG_ID_PLAYER_CREATE_REQUEST:
				{
					handle_create_player(extern_data);
					break;
				}
				/*case DELETE_PLAYER_REQUEST:
				{
					handle_delete_player(extern_data);
					break;
				}*/
				case MSG_ID_ENTER_GAME_REQUEST:
				{
					handle_enter_game(extern_data);
					break;
				}
			}
		}
		
		if (ret < 0) {
			LOG_INFO("%s: connect closed from fd %u, err = %d", __FUNCTION__, fd, errno);
			exit(0);
			return (-1);		
		} else if (ret > 0) {
			break;
		}
		
		ret = remove_one_buf();
	}
	return (0);
}

void cb_on_timer(evutil_socket_t, short, void *arg) {
	while (sg_filter_map_locker==1)
		usleep(1000);
	
	//uint32_t now = time(NULL);
	//std::vector<uint32_t>  filter_list;
	//std::map<uint32_t, std::pair<uint32_t, uint32_t> >::iterator it = sg_filter_list.begin();
	//for (; it!=sg_filter_list.end(); ++it) {
	//	if (0==it->second.first || (now-it->second.second)/86400 < it->second.first ) {
	//		filter_list.push_back(it->first);
	//	}		
	//}

	//const uint32_t send_max_size = 50;
	//for (size_t i=0; i<filter_list.size(); i+=send_max_size) {
	//	uint32_t sz = std::min(filter_list.size()-i, (size_t)send_max_size);

	//	PLAYER_TIREN_LIST_NOTIFY* notify = (PLAYER_TIREN_LIST_NOTIFY*)conn_node_loginsrv::get_send_buf(SERVER_PROTO_TIREN_LIST_NOTIFY, 0);
	//	notify->head.len = htons(sizeof(PLAYER_TIREN_LIST_NOTIFY));
	//	notify->count = sz;
	//	memcpy(notify->open_id, &filter_list[i], sizeof(uint32_t)*sz);

	//	EXTERN_DATA ext_data;
	//	ext_data.fd = 0;
	//	ext_data.open_id = 0;
	//	ext_data.player_id = 0;

	//	conn_node_loginsrv::add_extern_data(&notify->head, &ext_data);
	//	int ret = conn_node_loginsrv::connecter.send_one_msg(&notify->head, 1);
	//	if (ret != htons(notify->head.len)) {
	//		LOG_ERR("[%s : %d]: send data failed", __FUNCTION__, __LINE__);
	//	}
	//}

	add_timer(sg_timeout, &sg_event_timer, NULL);
}

