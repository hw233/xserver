#include "conn_node_clientsrv.h"
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
//#include <curl/curl.h>
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
#include "../proto/login.pb-c.h"
#include "move.pb-c.h"
#include "../proto/chat.pb-c.h"
#include "../proto/cast_skill.pb-c.h"
#include "../proto/pk.pb-c.h"
extern uint32_t sg_player_split;

conn_node_clientsrv conn_node_clientsrv::connecter;
//static char sql[1024*64];
static uint8_t send_buf[1024];
__attribute__((unused)) static PROTO_HEAD *head = (PROTO_HEAD *)&send_buf[0];
__attribute__((unused)) static uint8_t *send_data = &send_buf[sizeof(PROTO_HEAD)];
uint32_t sg_server_id = 0;

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

conn_node_clientsrv::conn_node_clientsrv()
{
	max_buf_len = 1024 * 1024;
	buf = (uint8_t *)malloc(max_buf_len + sizeof(EXTERN_DATA));
	assert(buf);

	posx = -1;	//玩家当前x坐标
	posz = -1;	//玩家当前z坐标
	move_lag = 0; //移动标记
	job = 0;	//职业
	chat_flag = 0;
	first_use_skill = 0;
	only_one_move = 0;
	first_borth = 0;
	pk_modle = 0;
	interval_time = 0;
}

conn_node_clientsrv::~conn_node_clientsrv()
{
}

int conn_node_clientsrv::recv_func(evutil_socket_t fd)
{
	//EXTERN_DATA *extern_data;
	//PROTO_HEAD *head;	
	for (;;) {
		int ret = get_one_buf();
		if (ret == 0) {
			//head = (PROTO_HEAD *)buf_head();
			//extern_data = get_extern_data(head);
			int cmd = get_cmd();
			//LOG_DEBUG("[%s:%d] cmd: %u, len:%u, seq:%u, player_id: %lu", __FUNCTION__, __LINE__, cmd, ENDION_FUNC_4(head->len), ENDION_FUNC_2(head->seq), player_id);
			switch(cmd)
			{
				case MSG_ID_LOGIN_ANSWER:
					handle_login_answer();
					break;
				case MSG_ID_PLAYER_LIST_ANSWER:
					handle_player_list_answer();
					break;
				case MSG_ID_PLAYER_CREATE_ANSWER:
					handle_player_create_answer();
					break;
				case MSG_ID_ENTER_GAME_ANSWER:
					handle_enter_game_answer();
					break;
				case MSG_ID_MOVE_ANSWER:			//请求移动服务器返回
					handle_player_move_answer();
					break;
				case MSG_ID_SKILL_CAST_NOTIFY: //施法应答
					handle_player_useskill_answer();
					break;
				case MSG_ID_SKILL_HIT_NOTIFY:	//技能命中通知
					handle_player_shill_mingzhong_answer();
					break;
				/*case MSG_ID_CHAT_NOTIFY:
					send_chat_info_answer();
					break;*/
				//case MSG_ID_ENTER_RAID_NOTIFY:	//进入新手副本通知
				//	LOG_INFO("进入新手副本通知到了");
					//send_chat_info_request();
					//send_move_request();
					//break;

				case MSG_ID_SIGHT_CHANGED_NOTIFY:  //视野变化通知
					handle_player_sight_changed_answer();
					break;
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


static bool get_login_info(int channel, uint32_t openId, std::string& szOpenId, std::string& szKey) {
	char szUrl[1024] = { 0 };
	char outStr[2048] = { 0 };

	memset(outStr, 0, sizeof(outStr));
	snprintf(szUrl, sizeof(szUrl), "192.168.2.114/cgi-bin/cgi_game_login?channel=%u&open_id=%u", channel, openId);

	CURL* curl_obj = curl_easy_init();	
	if (!curl_obj) {
		LOG_ERR("[%s:%d] curl_easy_init failed.", __FUNCTION__, __LINE__);
		return false;	
	}

	curl_easy_setopt(curl_obj, CURLOPT_URL, szUrl);
	curl_easy_setopt(curl_obj, CURLOPT_TIMEOUT, 5);
	curl_easy_setopt(curl_obj, CURLOPT_WRITEFUNCTION, &__curl_process_data__);
	curl_easy_setopt(curl_obj, CURLOPT_WRITEDATA, outStr);

	int ret = curl_easy_perform(curl_obj);
	if (0 != ret) {
		LOG_ERR("[%s:%d] call curl_easy_perform failed, ret:%d", __FUNCTION__, __LINE__, ret);

		curl_easy_cleanup(curl_obj);
		return false;	
	}

	curl_easy_cleanup(curl_obj);

	json_object *new_obj = json_tokener_parse(outStr);

	if (!new_obj) {
		LOG_ERR("[%s:%d] resp is not json, channel:%d, open_id:%u", __FUNCTION__, __LINE__, channel, openId);
		return false;
	}

	std::map<std::string, std::string>  jsonMap;
	json_object_object_foreach(new_obj, key, val) 
	{
		jsonMap[std::string(key)] = std::string(json_object_to_json_string(val));
	}

	std::map<std::string, std::string>::iterator it;
	it = jsonMap.find("code");
	if (it == jsonMap.end())
		return false;
	
	int code = atoi(it->second.c_str());
	if (code != 0)
	{
		LOG_ERR("[%s:%d] get login info failed, code:%d", __FUNCTION__, __LINE__, code);
		return false;	
	}
	
	it = jsonMap.find("open_id");
	if (it == jsonMap.end()) {
		LOG_ERR("[%s:%d] get login info failed, no open_id", __FUNCTION__, __LINE__);
		json_object_put(new_obj);
		return false;
	}

	szOpenId.assign(it->second.c_str()+1, it->second.length()-2);

	it = jsonMap.find("key");
	if (it == jsonMap.end()) {
		LOG_ERR("[%s:%d] get login info failed, no key", __FUNCTION__, __LINE__);
		json_object_put(new_obj);
		return false;
	}

	szKey.assign(it->second.c_str()+1, it->second.length()-2);

	json_object_put(new_obj);

	return true;
}

int conn_node_clientsrv::send_login_request(uint32_t open_id)
{
	int channel = 1;
	std::string szOpenId, szKey;
	if (get_login_info(channel, open_id, szOpenId, szKey) == false)
	{
		return -1;
	}

	LoginRequest req;
	login_request__init(&req);
	req.channel = channel;
	req.openid = const_cast<char*>(szOpenId.c_str());
	req.key = const_cast<char*>(szKey.c_str());
	
	size_t size = login_request__pack(&req, get_send_data());
	if (size != (size_t)-1)
	{
		PROTO_HEAD* head = get_send_buf(MSG_ID_LOGIN_REQUEST, seq++);
		head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD) + size);
		int ret = this->send_one_msg(head, 1);
		if (ret != (int)ENDION_FUNC_4(head->len))
		{
			LOG_ERR("[%s:%d] send to server failed err[%d]", __FUNCTION__, __LINE__, errno);
			return -1;
		}
	}

	return 0;
}

int conn_node_clientsrv::send_player_list_request(void)
{
	PROTO_HEAD* head = get_send_buf(MSG_ID_PLAYER_LIST_REQUEST, seq++);
	head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD));
	int ret = this->send_one_msg(head, 1);
	if (ret != (int)ENDION_FUNC_4(head->len))
	{
		LOG_ERR("[%s:%d] send to server failed err[%d]", __FUNCTION__, __LINE__, errno);
		return -1;
	}
	return 0;
}

int conn_node_clientsrv::send_player_create_request(uint32_t job, std::string name)
{
	PlayerCreateRequest req;
	player_create_request__init(&req);
	req.job = job;
	req.name = const_cast<char*>(name.c_str());
	
	size_t size = player_create_request__pack(&req, get_send_data());
	if (size != (size_t)-1)
	{
		PROTO_HEAD* head = get_send_buf(MSG_ID_PLAYER_CREATE_REQUEST, seq++);
		head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD) + size);
		int ret = this->send_one_msg(head, 1);
		if (ret != (int)ENDION_FUNC_4(head->len))
		{
			LOG_ERR("[%s:%d] send to server failed err[%d]", __FUNCTION__, __LINE__, errno);
			return -1;
		}
	}
	return 0;
}

int conn_node_clientsrv::send_enter_game_request(uint64_t player_id)
{
	EnterGameRequest req;
	enter_game_request__init(&req);
	req.playerid = player_id;
	
	size_t size = enter_game_request__pack(&req, get_send_data());
	if (size != (size_t)-1)
	{
		PROTO_HEAD* head = get_send_buf(MSG_ID_ENTER_GAME_REQUEST, seq++);
		head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD) + size);
		int ret = this->send_one_msg(head, 1);
		if (ret != (int)ENDION_FUNC_4(head->len))
		{
			LOG_ERR("[%s:%d] send to server failed err[%d]", __FUNCTION__, __LINE__, errno);
			return -1;
		}
	}
	return 0;
}

int conn_node_clientsrv::handle_login_answer(void)
{
	LoginAnswer *resp;
	resp = login_answer__unpack(NULL, get_len() - sizeof(PROTO_HEAD), (uint8_t *)get_data());
	if (!resp) {
		LOG_ERR("[%s:%d] unpack LoginAnswer failed, open_id:%u, player_id:%lu", __FUNCTION__, __LINE__, open_id, player_id);
		return -1;
	}

	int result = resp->result;
	int nopen_id = resp->openid;
	
	login_answer__free_unpacked(resp, NULL);
	
	LOG_DEBUG("[%s:%d] login result:%d, open_id:%d", __FUNCTION__, __LINE__, result, nopen_id);
	if (result == 0)
	{
		this->open_id = nopen_id;
		send_player_list_request();
	}

	return 0;
}

int conn_node_clientsrv::handle_player_list_answer(void)
{
	PlayerListAnswer *resp;
	uint32_t type = 0;
	resp = player_list_answer__unpack(NULL, get_len() - sizeof(PROTO_HEAD), (uint8_t *)get_data());
	if (!resp) {
		LOG_ERR("[%s:%d] unpack failed, open_id:%u, player_id:%lu", __FUNCTION__, __LINE__, open_id, player_id);
		return -1;
	}

	int result = resp->result;
	uint64_t player_id = 0;
	if (resp->n_playerlist > 0)
	{
		player_id = resp->playerlist[0]->playerid;
	}
	LOG_DEBUG("[%s:%d] result:%d, list_size:%lu, player_id:%lu", __FUNCTION__, __LINE__, result, resp->n_playerlist, player_id);
	
	player_list_answer__free_unpacked(resp, NULL);
	
	if (result == 0)
	{
		if (player_id > 0 )
		{
			this->player_id = player_id;
			send_enter_game_request(player_id);
		}
		else
		{
			char player_name[30];
			sprintf(player_name, "%u_%u", open_id, 1);
			type = 1;
			first_borth = 1;
			send_player_create_request(type, player_name);

		}
	}

	return 0;
}

int conn_node_clientsrv::handle_player_create_answer(void)
{
	PlayerCreateAnswer *resp;
	resp = player_create_answer__unpack(NULL, get_len() - sizeof(PROTO_HEAD), (uint8_t *)get_data());
	if (!resp) {
		LOG_ERR("[%s:%d] unpack failed, open_id:%u, player_id:%lu", __FUNCTION__, __LINE__, open_id, player_id);
		return -1;
	}

	int result = resp->result;
	uint64_t player_id = 0;
	if (resp->playerlist)
	{
		player_id = resp->playerlist->playerid;
	}
	LOG_DEBUG("[%s:%d] result:%d, player_id:%lu", __FUNCTION__, __LINE__, result, player_id);
	
	player_create_answer__free_unpacked(resp, NULL);
	
	if (result == 0)
	{
		if (player_id > 0)
		{
			send_enter_game_request(player_id);
			this->player_id = player_id;
		}
	}

	return 0;
}

int conn_node_clientsrv::handle_enter_game_answer(void)
{
	EnterGameAnswer *resp;
	resp = enter_game_answer__unpack(NULL, get_len() - sizeof(PROTO_HEAD), (uint8_t *)get_data());
	if (!resp) {
		LOG_ERR("[%s:%d] unpack failed, open_id:%u, player_id:%lu", __FUNCTION__, __LINE__, open_id, player_id);
		return -1;
	}

	int result = resp->result;
	if (result != 0)
	{
		this->player_id = 0;
	}
	LOG_DEBUG("[%s:%d] result:%d, player_id:%lu", __FUNCTION__, __LINE__, result, player_id);
	
	enter_game_answer__free_unpacked(resp, NULL);
	
	if (result == 0)
	{
		this->posx = resp->posx;
		this->posz = resp->posz;
		PROTO_HEAD* head = get_send_buf(MSG_ID_ENTER_SCENE_READY_REQUEST, 0);
		head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD));

		int ret = this->send_one_msg(head, 1);
		if (ret != (int)ENDION_FUNC_4(head->len))
		{
			LOG_ERR("通知后台已经准备好加入场景失败");
			return -1;
		}

		if (sg_player_split == 0)
		{
			borth_send_move_request();
		}
		
		//升级设置pk模式
		send_chat_info_request();
		send_chat_info_request();
		handle_player_setpkmodle_request();
	}

	return 0;
}

int conn_node_clientsrv::send_move_request()
{
//	sleep(2);
	if(posx==-1 || posz==-1)
	{
		LOG_ERR("[%s:%d]让玩家随机移动失败，未能获取玩家当前位置", __FUNCTION__, __LINE__);
		return -1;
	}
	size_t size ;
	MoveRequest req;
	size_t i = 0;
	move_request__init(&req);
	req.n_data = 3;
	PosData data[req.n_data];
	PosData *pdata[req.n_data];

	
	if (move_lag == 0)
	{
		move_lag = 1;
		for (i = 0; i < req.n_data; i++)
		{
			pos_data__init(&data[i]);
			pdata[i] = &data[i];
			data[i].pos_x = posx + i ;
			data[i].pos_z = posz + i ;
			LOG_DEBUG("让玩家随机移动成功，玩家openid[%u]移动的第[%lu]步位置posx[%f],posz[%f]", open_id, i, data[i].pos_x, data[i].pos_z);
		}
	}
	else
	{
		move_lag = 0;
		for (i = 0; i < req.n_data; i++)
		{
			pos_data__init(&data[i]);
			pdata[i] = &data[i];
			data[i].pos_x = posx - i;
			data[i].pos_z = posz - i;
			LOG_DEBUG("让玩家随机移动成功，玩家openid[%u]移动的第[%lu]步位置posx[%f],posz[%f]", open_id, i, data[i].pos_x, data[i].pos_z);
		}
	}
	
	posx = data[i - 1].pos_x;
	posz = data[i - 1].pos_z;
	req.data = &pdata[0];
	size = move_request__pack(&req, get_send_data());
	if (size != (size_t)-1)
	{
		PROTO_HEAD* head = get_send_buf(MSG_ID_MOVE_REQUEST, seq++);
		head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD) + size);
		int ret = this->send_one_msg(head, 1);
		if (ret != (int)ENDION_FUNC_4(head->len))
		{
			LOG_ERR("[%s:%d] send to server failed err[%d]", __FUNCTION__, __LINE__, errno);
			return -1;
		}
	}
	return(0);
}
//出生的时候发送一次移动请求定点移动，让玩家分开站
int conn_node_clientsrv::borth_send_move_request()
{
	if (first_borth == 0)
	{
		LOG_DEBUG("[%s:%d] 请求分开站立失败，不是第一次创建角色", __FUNCTION__, __LINE__);
		return -1;
	}
	size_t size;
	MoveRequest req;
	size_t i = 0;
	move_request__init(&req);
	req.n_data = 1;
	PosData data[req.n_data];
	PosData *pdata[req.n_data];

	if (open_id < 100)
	{
		for (i = 0; i < req.n_data; i++)
		{
			pos_data__init(&data[i]);
			pdata[i] = &data[i];
			
			data[i].pos_x = 362 + open_id;
			data[i].pos_z = 72;
		}
	}
	else if (open_id >= 100 && open_id < 200)
	{
		for (i = 0; i < req.n_data; i++)
		{
			pos_data__init(&data[i]);
			pdata[i] = &data[i];
			data[i].pos_x = 362 - open_id % 100;
			data[i].pos_z = 72;
		}
	}
	else if (open_id >= 200 && open_id < 300)
	{
		for (i = 0; i < req.n_data; i++)
		{
			pos_data__init(&data[i]);
			pdata[i] = &data[i];
			
			data[i].pos_x = 362;
			data[i].pos_z = 72 + open_id % 200;
		}
	}
	else if (open_id >= 300 && open_id < 400)
	{
		for (i = 0; i < req.n_data; i++)
		{
			pos_data__init(&data[i]);
			pdata[i] = &data[i];
			
			data[i].pos_x = 362;
			data[i].pos_z = 72 - open_id % 300;
		}
	}
	else if (open_id >= 400 && open_id < 500)
	{
		for (i = 0; i < req.n_data; i++)
		{
			pos_data__init(&data[i]);
			pdata[i] = &data[i];

			data[i].pos_x = 362 - open_id % 400;
			data[i].pos_z = 72 - open_id % 400;
		}
	}
	else if (open_id >= 500 && open_id < 600)
	{
		for (i = 0; i < req.n_data; i++)
		{
			pos_data__init(&data[i]);
			pdata[i] = &data[i];

			data[i].pos_x = 362 + open_id % 500;
			data[i].pos_z = 72 + open_id % 500;
		}
	}
	else if (open_id >= 600 && open_id < 700)
	{
		for (i = 0; i < req.n_data; i++)
		{
			pos_data__init(&data[i]);
			pdata[i] = &data[i];

			data[i].pos_x = 362 + open_id % 600;
			data[i].pos_z = 72 - open_id % 600;
		}
	}
	else if (open_id >= 700 && open_id < 800)
	{
		for (i = 0; i < req.n_data; i++)
		{
			pos_data__init(&data[i]);
			pdata[i] = &data[i];

			data[i].pos_x = 362 - open_id % 700;
			data[i].pos_z = 72 + open_id % 700;
		}
	}
	else
	{
		LOG_DEBUG("[%s:%d] 角色超过800个的安排到出生点", __FUNCTION__, __LINE__);
		return -1;
	}


	posx = data[i - 1].pos_x;
	posz = data[i - 1].pos_z;
	req.data = &pdata[0];
	size = move_request__pack(&req, get_send_data());
	if (size != (size_t)-1)
	{
		PROTO_HEAD* head = get_send_buf(MSG_ID_MOVE_REQUEST, seq++);
		head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD) + size);
		int ret = this->send_one_msg(head, 1);
		if (ret != (int)ENDION_FUNC_4(head->len))
		{
			LOG_ERR("[%s:%d] send to server failed err[%d]", __FUNCTION__, __LINE__, errno);
			return -1;
		}
	}
	return(0);

}

int conn_node_clientsrv::handle_player_move_answer()
{
	CommAnswer *resp;
	resp = comm_answer__unpack(NULL, get_len() - sizeof(PROTO_HEAD), (uint8_t *)get_data());
	int32_t result = resp->result;
	comm_answer__free_unpacked(resp, NULL);
	if (result == 0)
	{
		LOG_DEBUG("让玩家随机移动成功，玩家openid[%u]当前位置posx[%f],posz[%f]", open_id, posx, posz);
	}
	return -1;
}
//通过聊天发送GM命令给玩家升级，并且给嗜血道具
int conn_node_clientsrv::send_chat_info_request()
{
	Chat resp;
	chat__init(&resp);
	resp.channel = CHANNEL__area;
	if (chat_flag == 0)
	{
		chat_flag = 2;
		resp.contain = new char[strlen("add prop 201070037 10")];
		memcpy(resp.contain, "add prop 201070037 10", strlen("add prop 201070037 10"));
		resp.contain[strlen("add prop 201070037 10")] = '\0';
		
	}
	else
	{
		resp.contain = new char[strlen("add exp 10000000")];
		memcpy(resp.contain, "add exp 10000000", strlen("add exp 10000000"));
		resp.contain[strlen("add exp 10000000")] = '\0';
	}
	resp.sendplayerid = player_id;
	size_t size = chat__pack(&resp, get_send_data());

	if (size != (size_t)-1)
	{
		PROTO_HEAD* head = get_send_buf(MSG_ID_CHAT_REQUEST, seq++);
		head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD) + size);
		int ret = this->send_one_msg(head, 1);
		if (ret != (int)ENDION_FUNC_4(head->len))
		{
			LOG_ERR("[%s:%d] send to server failed err[%d]", __FUNCTION__, __LINE__, errno);
			return -1;
		}
	}
	delete[] resp.contain;
	
	return(0);
}
int conn_node_clientsrv::handle_player_useskill_request()
{
	
	SkillCastRequest resp;
	skill_cast_request__init(&resp);
	
	resp.cur_pos = new PosData[sizeof(PosData)];
	pos_data__init(resp.cur_pos);
	resp.cur_pos->pos_x = posx;
	resp.cur_pos->pos_z = posz;
	
	resp.skillid = 111100101;
	resp.direct_x = -0.85567981;
	resp.direct_z = -0.517505646;
	size_t size = skill_cast_request__pack(&resp, get_send_data());
	
	if (size != (size_t)-1)
	{
		PROTO_HEAD* head = get_send_buf(MSG_ID_SKILL_CAST_REQUEST, seq++);
		head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD) + size);
		int ret = this->send_one_msg(head, 1);
		if (ret != (int)ENDION_FUNC_4(head->len))
		{
			LOG_ERR("[%s:%d] send to server failed err[%d]", __FUNCTION__, __LINE__, errno);
			return -1;
		}
	}
	return(0);
}

int conn_node_clientsrv::handle_player_useskill_answer()
{
	uint64_t shifa_player_id;
	SkillCastNotify *resp;
	resp = skill_cast_notify__unpack(NULL, get_len() - sizeof(PROTO_HEAD), (uint8_t *)get_data());
	if (resp == NULL)
	{
		LOG_ERR("[%s:%d] usekill answer fail, resp is null", __FUNCTION__, __LINE__);
		return -1;
	}
	shifa_player_id = resp->playerid;
	skill_cast_notify__free_unpacked(resp, NULL);
	
	if (shifa_player_id != player_id)
	{
		LOG_DEBUG("[%s:%d] 技能施法玩家非本玩家", __FUNCTION__, __LINE__);
		return -1;
	}
 	handle_player_mingzhong_request();

	return 0;
}
int conn_node_clientsrv::handle_player_mingzhong_request()
{
	size_t i = 0;
	SkillHitRequest req;
	uint64_t target_playerid = 0; //被打击这id
	SightPlayerBaseInfo* target_player = NULL; //被打击者
	skill_hit_request__init(&req);
	PosData ndata;
	pos_data__init(&ndata);
	ndata.pos_x = posx;
	ndata.pos_z = posz;
	req.attack_pos = &ndata;
	req.n_target_playerid = 1;
	if (sight_player.size() != 0)
	{
		std::map<uint64_t, SightPlayerBaseInfo*>::iterator itr = sight_player.begin();
		for (;itr != sight_player.end(); itr++)
		{
			if (itr->first == 47244643727)
				continue;
			if (itr->second->hp <= 0)
			{
				continue;
			}
			else
			{
				target_playerid = itr->first;
				target_player = itr->second;
				break;
			}
			
		}	
		
	}
		
	if (target_playerid == 0 || target_player == NULL)
	{
		LOG_ERR("%s,%d选定命中目标失败", __FUNCTION__, __LINE__);
		return -1;
	}

	req.target_playerid = &target_playerid;
	req.n_target_pos = 1;
	PosData data[req.n_target_pos];
	PosData *pdata[req.n_target_pos];

	for (; i < req.n_target_pos; i++)
	{
		pos_data__init(&data[i]);
		pdata[i] = &data[i];
		data[i].pos_x = target_player->data[i]->pos_x;
		data[i].pos_z = target_player->data[i]->pos_z;
	}
	req.target_pos = &pdata[0];
	
	size_t size = skill_hit_request__pack(&req, get_send_data());

	if (size != (size_t)-1)
	{
		PROTO_HEAD* head = get_send_buf(MSG_ID_SKILL_HIT_REQUEST, seq++);
		head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD) + size);
		int ret = this->send_one_msg(head, 1);
		if (ret != (int)ENDION_FUNC_4(head->len))
		{
			LOG_ERR("[%s:%d] send to server failed err[%d]", __FUNCTION__, __LINE__, errno);
			return -1;
		}
	}
	return(0);

}
//聊天回复
int conn_node_clientsrv::send_chat_info_answer()
{
	//handle_player_useskill_request();
	Chat *resp;
	resp = chat__unpack(NULL, get_len() - sizeof(PROTO_HEAD), (uint8_t*)get_data());
	if (resp)
	{
		chat__free_unpacked(resp,NULL);
	}
	return(0);
}

//请求设置PK模式
int conn_node_clientsrv::handle_player_setpkmodle_request()
{
	SetPkTypeRequest resp;
	set_pk_type_request__init(&resp);
	resp.pk_type = 2;
	size_t size = set_pk_type_request__pack(&resp, get_send_data());
	if (size != (size_t)-1)
	{
		PROTO_HEAD* head = get_send_buf(MSG_ID_SET_PK_TYPE_REQUEST, seq++);
		head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD) + size);
		int ret = this->send_one_msg(head, 1);
		if (ret != (int)ENDION_FUNC_4(head->len))
		{
			LOG_ERR("[%s:%d] send to server failed err[%d]", __FUNCTION__, __LINE__, errno);
			return -1;
		}
	}
	return(0);
}

int conn_node_clientsrv::handle_player_sight_changed_answer()
{
	SightChangedNotify* notify;

	notify = sight_changed_notify__unpack(NULL, get_len() - sizeof(PROTO_HEAD), (uint8_t*)get_data());

	if (notify == NULL)
	{
		LOG_ERR("视野变化解包为空");
		return -1;
	}
	LOG_DEBUG("视野有变化的玩家id[%lu],视野增加的人数[%lu]", this->player_id,notify->n_add_player);
	if (notify->n_add_player > 0)
	{
		for (size_t i = 0; i < notify->n_add_player; i++)
		{
			if (sight_player.find(notify->add_player[i]->playerid) == sight_player.end())
			{
				sight_player.insert(std::make_pair(notify->add_player[i]->playerid, notify->add_player[i]));
			}
		}

	}

	if (notify->n_delete_player > 0)
	{
		for (size_t j = 0; j < notify->n_delete_player; j++)
		{
			std::map<uint64_t, SightPlayerBaseInfo*>::iterator itr = sight_player.find(notify->delete_player[j]);
			if (itr != sight_player.end())
			{
				sight_player.erase(itr);
			}	
		}
		
	}
	
	return 0;
}

int conn_node_clientsrv::handle_player_shill_mingzhong_answer()
{
	SkillHitNotify* notify;

	notify = skill_hit_notify__unpack(NULL, get_len() - sizeof(PROTO_HEAD), (uint8_t*)get_data());

	if (notify == NULL)
	{
		LOG_ERR("%s,%d技能命中返回信息有误", __FUNCTION__, __LINE__);
		return -1;
	}
	if (notify->n_target_player > 0)
	{
		for (size_t i =0; i < notify->n_target_player; i++)
		{
			std::map<uint64_t, SightPlayerBaseInfo*>::iterator itr = sight_player.find(notify->target_player[i]->playerid);
			if (itr != sight_player.end())
			{
				itr->second->hp = notify->target_player[i]->cur_hp;
			}
		}
	}
	return 0;
}

