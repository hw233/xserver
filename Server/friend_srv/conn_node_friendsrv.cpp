#include "conn_node_friendsrv.h"
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
#include "server_proto.h"
#include <vector>
#include "msgid.h"
#include "error_code.h"
#include "attr_id.h"
#include "cgi_common.h"
#include "comm_message.pb-c.h"
#include "team.pb-c.h"
#include "wanyaogu.pb-c.h"
#include "tower.pb-c.h"
#include "hotel.pb-c.h"
#include "zhenying.pb-c.h"
#include "player_redis_info.pb-c.h"
#include "app_data_statis.h"
#include "friend_util.h"
#include "friend.pb-c.h"
#include <algorithm>
#include "chat.pb-c.h"
#include "bag.pb-c.h"
#include "setting.pb-c.h"
#include "friend_config.h"
#include "role.pb-c.h"
#include "redis_util.h"

//static const int MIN_ZHENYING_PLAYER_NUM = 500;

typedef size_t(*pack_func)(const void *message, uint8_t *out);

extern int send_mail(conn_node_base *connecter, uint64_t player_id, uint32_t type,
	char *title, char *sender_name, char *content, std::vector<char *> *args,
	std::map<uint32_t, uint32_t> *attachs, uint32_t statis_id);
extern char sg_player_cache_key[];
extern void set_proto_friend(PlayerRedisInfo &player, FriendPlayerBriefData &proto_data);
extern int save_friend_chat(uint64_t player_id, ChatList *chats);
extern ChatList *load_friend_chat(uint64_t player_id);
extern void delete_friend_chat(uint64_t player_id);
extern void add_friend_offline_chat(uint64_t player_id, Chat *chat);
extern void add_friend_offline_system(uint64_t player_id, SystemNoticeNotify *sys);
extern void add_friend_offline_gift(uint64_t player_id, FriendGiftData *gift);

void send_friend_to_game(EXTERN_DATA *extern_data, uint16_t msg_id, void *data, pack_func func)
{
	PROTO_HEAD *proto_head;
	PROTO_HEAD *real_head;
	proto_head = (PROTO_HEAD *)conn_node_base::global_send_buf;
	proto_head->msg_id = ENDION_FUNC_2(SERVER_PROTO_FRIEND_TO_GAME);
	proto_head->seq = 0;

	real_head = (PROTO_HEAD *)proto_head->data;
	size_t size = func(data, (uint8_t *)real_head->data);
	real_head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD)+size);
	real_head->msg_id = ENDION_FUNC_2(msg_id);
	conn_node_friendsrv::connecter.add_extern_data(real_head, extern_data);

	proto_head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD)+real_head->len);

	if (conn_node_friendsrv::connecter.send_one_msg(proto_head, 1) != (int)(ENDION_FUNC_4(proto_head->len))) {
		LOG_ERR("%s %d: send to all failed err[%d]", __FUNCTION__, __LINE__, errno);
	}
}


#define MAX_WANYAOKA_PER_PLAYER 128

conn_node_friendsrv conn_node_friendsrv::connecter;
char conn_node_friendsrv::server_key[64];
char conn_node_friendsrv::server_wyk_key[64];
char conn_node_friendsrv::tower_cd_key[64];
char conn_node_friendsrv::zhenying_key[64] = "zhenying";
char conn_node_friendsrv::tower_max_key[64] = "tower_max";

char sg_str_server_id[64];
uint32_t sg_server_id = 0;
CRedisClient sg_redis_client;
struct event sg_event_timer;
struct timeval sg_timeout = { 300, 0 };


conn_node_friendsrv::conn_node_friendsrv()
{
	max_buf_len = 1024 * 1024;
	buf = (uint8_t *)malloc(max_buf_len + sizeof(EXTERN_DATA));
	assert(buf);
}

conn_node_friendsrv::~conn_node_friendsrv()
{
}

int calc_zhenying_power_rate(uint64_t manLeft, uint64_t powerLeft, uint64_t manRight, uint64_t powerRight)
{
	ParameterTable * config = get_config_by_id(161000171, &parameter_config);
	if (config == NULL)
	{
		return 50;
	}
	uint64_t rateLeft = config->parameter1[0] * manLeft + config->parameter1[1] * powerLeft;
	uint64_t rateRight = config->parameter1[0] * manRight + config->parameter1[1] * powerRight;
	uint64_t rateFinal = 50;
	if (rateLeft >= rateRight)
	{
		rateFinal = 50 + config->parameter1[3] * rateLeft / (rateLeft + rateRight + config->parameter1[2]);
	}
	else
	{
		rateFinal = 50 + config->parameter1[3] * rateRight / (rateLeft + rateRight + config->parameter1[2]);
		rateFinal = 100 - rateFinal;
	}
	return rateFinal;
}
int check_can_join_zhenying(uint32_t rate, uint32_t &gold, uint32_t free)
{
	ParameterTable * config = get_config_by_id(161000129, &parameter_config);
	if (config == NULL)
	{
		return 1;
	}

	if (rate >= config->parameter1[0])
	{
		return 190500189;
	}
	std::map<uint64_t, struct CampTable*>::iterator it = zhenying_base_config.begin();
	CampTable * table = it->second;
	if (gold < table->Consume * rate / 100 && free < 1)
	{
		return 190400005;
	}
	gold = table->Consume * rate / 100;
	return 0;
}

void conn_node_friendsrv::handle_chose_zhenying()
{
	PROTO_HEAD *head = get_head();
	EXTERN_DATA *extern_data = get_extern_data(head);

	int data_len = ENDION_FUNC_4(head->len) - sizeof(PROTO_HEAD)-sizeof(EXTERN_DATA);

	AddZhenyingPlayer *req = add_zhenying_player__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return;
	}

	data_len = MAX_GLOBAL_SEND_BUF;
	int ret = sg_redis_client.hget_bin(zhenying_key, server_key, (char *)conn_node_base::global_send_buf, &data_len);
	ZhenyingRedis *rzhenying = NULL;
	ZhenyingRedis send;
	bool release = false;
	int result = 0;
	if (ret < 0)
	{
		zhenying_redis__init(&send);
		rzhenying = &send;
	}
	else
	{
		rzhenying = zhenying_redis__unpack(NULL, data_len, conn_node_base::global_send_buf);

		if (rzhenying == NULL)
		{
			zhenying_redis__init(&send);
			rzhenying = &send;
		}
		else
		{
			release = true;
		}
	}

	uint32_t rate = calc_zhenying_power_rate(rzhenying->man_fulongguo, rzhenying->power_fulongguo, rzhenying->man_wanyaogu, rzhenying->power_wanyaogu);
	if (req->zhenying != ZHENYING__TYPE__FULONGGUO)
	{
		rate = 100 - rate;//calc_zhenying_power_rate(rzhenying->man_fulongguo, rzhenying->power_fulongguo, rzhenying->man_wanyaogu, rzhenying->power_wanyaogu);
	}
	//else
	//{
	//	rate = calc_zhenying_power_rate(rzhenying->man_wanyaogu, rzhenying->power_wanyaogu, rzhenying->man_fulongguo, rzhenying->power_fulongguo);
	//}
	result = check_can_join_zhenying(rate, req->gold, req->free);
	if (result == 0)
	{
		if (req->zhenying == ZHENYING__TYPE__FULONGGUO)
		{
			++rzhenying->man_fulongguo;
			rzhenying->power_fulongguo += req->fighting_capacity;
		}
		else
		{
			++rzhenying->man_wanyaogu;
			rzhenying->power_wanyaogu += req->fighting_capacity;
		}
	}

	data_len = zhenying_redis__pack(rzhenying, (uint8_t *)conn_node_base::global_send_buf);
	ret = sg_redis_client.hset_bin(zhenying_key, server_key, (char *)conn_node_base::global_send_buf, data_len);
	if (ret < 0)
	{
		LOG_ERR("%s: oper failed, ret = %d", __FUNCTION__, ret);
	}
	if (release)
	{
		zhenying_redis__free_unpacked(rzhenying, NULL);
	}

	head->msg_id = SERVER_PROTO_CHOSE_ZHENYING_REQUEST;
	req->ret = result;
	send_friend_to_game(extern_data, head->msg_id, req, (pack_func)add_zhenying_player__pack);
	add_zhenying_player__free_unpacked(req, NULL);

	LOG_DEBUG("%s: team info %lu len[%d] ret", __FUNCTION__, extern_data->player_id, data_len);
}

void conn_node_friendsrv::handle_zhenying_power()
{
	PROTO_HEAD *head = get_head();
	EXTERN_DATA *extern_data = get_extern_data(head);

	int data_len = ENDION_FUNC_4(head->len) - sizeof(PROTO_HEAD)-sizeof(EXTERN_DATA);

	AddZhenyingPlayer *req = add_zhenying_player__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return;
	}

	data_len = MAX_GLOBAL_SEND_BUF;
	int ret = sg_redis_client.hget_bin(zhenying_key, server_key, (char *)conn_node_base::global_send_buf, &data_len);
	ZhenyingPower send;
	zhenying_power__init(&send);
	if (ret >= 0)
	{
		ZhenyingRedis *rzhenying = zhenying_redis__unpack(NULL, data_len, conn_node_base::global_send_buf);
		if (rzhenying != NULL)
		{
			send.power_fulongguo = rzhenying->power_fulongguo;
			send.power_wanyaogu = rzhenying->power_wanyaogu;

			send.man_fulongguo = calc_zhenying_power_rate(rzhenying->man_fulongguo, rzhenying->power_fulongguo, rzhenying->man_wanyaogu, rzhenying->power_wanyaogu);
			send.man_wanyaogu = 100 - send.man_fulongguo;

			//if (req->zhenying == ZHENYING__TYPE__FULONGGUO)
			//{
			//	send.power_man = rzhenying->power_man_fulongguo;
			//}
			//else if (req->zhenying == ZHENYING__TYPE__WANYAOGU)
			//{
			//	send.power_man = rzhenying->power_man_wanyaogu;
			//}
			//else
			//{
			//	send.power_man = 0;
			//}

			zhenying_redis__free_unpacked(rzhenying, NULL);
		}
	}
	else
	{
		//send.power_fulongguo = 0;
		//send.power_wanyaogu = 0;
		send.man_wanyaogu = 50;
		send.man_fulongguo = 50;
		//send.power_man = 0;
	}
	fast_send_msg(&conn_node_friendsrv::connecter, extern_data, MSG_ID_ZHENYING_POWER_ANSWER, zhenying_power__pack, send);
	add_zhenying_player__free_unpacked(req, NULL);

	LOG_DEBUG("%s: team info %lu len[%d] ret", __FUNCTION__, extern_data->player_id, data_len);
}

void conn_node_friendsrv::handle_zhenying_change_power()
{
	PROTO_HEAD *head = get_head();
	EXTERN_DATA *extern_data = get_extern_data(head);
	int data_len = ENDION_FUNC_4(head->len) - sizeof(PROTO_HEAD)-sizeof(EXTERN_DATA);

	PROTO_ZHENYIN_CHANGE_POWER *req = (PROTO_ZHENYIN_CHANGE_POWER *)get_data();
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return;
	}

	data_len = MAX_GLOBAL_SEND_BUF;
	int ret = sg_redis_client.hget_bin(zhenying_key, server_key, (char *)conn_node_base::global_send_buf, &data_len);
	if (ret >= 0)
	{
		ZhenyingRedis *rzhenying = zhenying_redis__unpack(NULL, data_len, conn_node_base::global_send_buf);
		if (rzhenying != NULL)
		{
			if (req->zhen_ying == ZHENYING__TYPE__FULONGGUO)
			{
				rzhenying->power_fulongguo += req->power;
			}
			else if (req->zhen_ying == ZHENYING__TYPE__WANYAOGU)
			{
				rzhenying->power_wanyaogu += req->power;
			}

			data_len = zhenying_redis__pack(rzhenying, (uint8_t *)conn_node_base::global_send_buf);
			ret = sg_redis_client.hset_bin(zhenying_key, server_key, (char *)conn_node_base::global_send_buf, data_len);
			if (ret < 0)
			{
				LOG_ERR("%s: oper failed, ret = %d", __FUNCTION__, ret);
			}

			zhenying_redis__free_unpacked(rzhenying, NULL);
		}
	}

	LOG_DEBUG("%s: team info %lu len[%d] ret", __FUNCTION__, extern_data->player_id, data_len);
}

// TODO: 这个函数可能会崩溃
void conn_node_friendsrv::handle_zhenying_add_kill()
{
	//LOG_DEBUG("%s: team info", __FUNCTION__);
	PROTO_HEAD *head = get_head();
	EXTERN_DATA *extern_data = get_extern_data(head);
	char key[128] = "zhenying";
	int data_len = ENDION_FUNC_4(head->len) - sizeof(PROTO_HEAD)-sizeof(EXTERN_DATA);

	AddZhenyingPlayer *req = add_zhenying_player__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return;
	}

	data_len = MAX_GLOBAL_SEND_BUF;
	int ret = sg_redis_client.hget_bin(server_key, key, (char *)conn_node_base::global_send_buf, &data_len);
	if (ret >= 0)
	{
		//LOG_DEBUG("%s: team info", __FUNCTION__);
		ZhenyingRedis *rzhenying = zhenying_redis__unpack(NULL, data_len, conn_node_base::global_send_buf);
		if (rzhenying != NULL)
		{
			ZhenyingRedis send;
			zhenying_redis__init(&send);
			//			bool save = false;
			char *save = NULL;
			if (req->zhenying == ZHENYING__TYPE__FULONGGUO)
			{
				if (req->kill > rzhenying->power_man_kill_fulongguo)
				{
					rzhenying->power_man_fulongguo = extern_data->player_id;
					save = rzhenying->power_name_fulongguo;
					rzhenying->power_name_fulongguo = req->name;
				}
			}
			else if (req->zhenying == ZHENYING__TYPE__WANYAOGU)
			{
				rzhenying->power_man_wanyaogu = extern_data->player_id;
				save = rzhenying->power_name_wanyaogu;
				rzhenying->power_name_wanyaogu = req->name;
			}

			if (save)
			{
				LOG_DEBUG("%s: save team info", __FUNCTION__);
				data_len = zhenying_redis__pack(rzhenying, (uint8_t *)conn_node_base::global_send_buf);
				ret = sg_redis_client.hset_bin(server_key, key, (char *)conn_node_base::global_send_buf, data_len);
				if (req->zhenying == ZHENYING__TYPE__FULONGGUO)
				{
					rzhenying->power_name_fulongguo = save;
				}
				else
				{
					assert(req->zhenying == ZHENYING__TYPE__WANYAOGU);
					rzhenying->power_name_wanyaogu = save;
				}


				if (ret < 0)
				{
					LOG_ERR("%s: oper failed, ret = %d", __FUNCTION__, ret);
				}//LOG_DEBUG("%s: team info", __FUNCTION__);
			}
			zhenying_redis__free_unpacked(rzhenying, NULL);
			LOG_DEBUG("%s: rzhenying != NULL, team info", __FUNCTION__);
		}
	}
	add_zhenying_player__free_unpacked(req, NULL);
	//LOG_DEBUG("%s: team info %lu len[%d] ret", __FUNCTION__, extern_data->player_id, data_len);
	LOG_DEBUG("%s: team info", __FUNCTION__);
}

void conn_node_friendsrv::handle_change_zhenying()
{
	PROTO_HEAD *head = get_head();
	EXTERN_DATA *extern_data = get_extern_data(head);

	int data_len = ENDION_FUNC_4(head->len) - sizeof(PROTO_HEAD)-sizeof(EXTERN_DATA);

	AddZhenyingPlayer *req = add_zhenying_player__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return;
	}

	data_len = MAX_GLOBAL_SEND_BUF;
	int ret = sg_redis_client.hget_bin(zhenying_key, server_key, (char *)conn_node_base::global_send_buf, &data_len);
	if (ret >= 0)
	{
		ZhenyingRedis *rzhenying = zhenying_redis__unpack(NULL, data_len, conn_node_base::global_send_buf);
		if (rzhenying != NULL)
		{
			uint32_t rate = calc_zhenying_power_rate(rzhenying->man_fulongguo, rzhenying->power_fulongguo, rzhenying->man_wanyaogu, rzhenying->power_wanyaogu);
			if (req->zhenying != ZHENYING__TYPE__FULONGGUO)
			{
				rate = 100 - rate;// calc_zhenying_power_rate(rzhenying->man_fulongguo, rzhenying->power_fulongguo, rzhenying->man_wanyaogu, rzhenying->power_wanyaogu);
			}
			//else
			//{
			//	rate = calc_zhenying_power_rate(rzhenying->man_wanyaogu, rzhenying->power_wanyaogu, rzhenying->man_fulongguo, rzhenying->power_fulongguo);
			//}
			int result = check_can_join_zhenying(rate, req->gold, req->free);
			if (result == 0)
			{
				if (req->zhenying == ZHENYING__TYPE__FULONGGUO)
				{
					++rzhenying->man_fulongguo;
					rzhenying->power_fulongguo += req->fighting_capacity;
					--rzhenying->man_wanyaogu;
					if (rzhenying->power_wanyaogu > req->fighting_capacity)
					{
						rzhenying->power_wanyaogu -= req->fighting_capacity;
					}
					else
					{
						rzhenying->power_wanyaogu = 0;
					}
				}
				else
				{
					--rzhenying->man_fulongguo;
					if (rzhenying->power_fulongguo > req->fighting_capacity)
					{
						rzhenying->power_fulongguo -= req->fighting_capacity;
					}
					else
					{
						rzhenying->power_fulongguo = 0;
					}

					++rzhenying->man_wanyaogu;
					rzhenying->power_wanyaogu += req->fighting_capacity;
				}

				data_len = zhenying_redis__pack(rzhenying, (uint8_t *)conn_node_base::global_send_buf);
				ret = sg_redis_client.hset_bin(zhenying_key, server_key, (char *)conn_node_base::global_send_buf, data_len);
				if (ret < 0)
				{
					LOG_ERR("%s: oper failed, ret = %d", __FUNCTION__, ret);
				}

			}
			head->msg_id = SERVER_PROTO_CHANGE_ZHENYING_REQUEST;
			req->ret = result;
			send_friend_to_game(extern_data, head->msg_id, req, (pack_func)add_zhenying_player__pack);
			zhenying_redis__free_unpacked(rzhenying, NULL);
		}
	}

	add_zhenying_player__free_unpacked(req, NULL);

	LOG_DEBUG("%s: team info %lu len[%d] ret", __FUNCTION__, extern_data->player_id, data_len);
}

void pack_team_mem_info(TeamMemInfo *mem, PlayerRedisInfo *rplayer)
{
	mem->icon = rplayer->head_icon;
	strcpy(mem->name, rplayer->name);
	mem->lv = rplayer->lv;
	mem->job = rplayer->job;
	mem->hp = rplayer->hp;
	mem->maxhp = rplayer->max_hp;
	mem->clothes = rplayer->clothes;
	mem->clothes_color_up = rplayer->clothes_color_up;
	mem->clothes_color_down = rplayer->clothes_color_down;
	mem->hat = rplayer->hat;
	mem->hat_color = rplayer->hat_color;
	mem->weapon = rplayer->weapon;
	mem->weapon_color = rplayer->weapon_color;
	mem->fight = rplayer->fighting_capacity;
	mem->zhenying = rplayer->zhenying;
	mem->head_icon = rplayer->head_icon;
	mem->guild = rplayer->guild_id;
	mem->sex = rplayer->sex;
}

//char allname[5 * 20 * 2][MAX_PLAYER_NAME_LEN];
void conn_node_friendsrv::handle_team_info()
{
	PROTO_HEAD *head = get_head();
	EXTERN_DATA *extern_data = get_extern_data(head);
	char key[128];
	int data_len = ENDION_FUNC_4(head->len) - sizeof(PROTO_HEAD)-sizeof(EXTERN_DATA);

	TeamInfo *req = team_info__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return;
	}
	for (size_t i = 0; i < req->n_mem; ++i)
	{
		if (req->mem[i]->online)
		{
			continue;
		}
		sprintf(key, "%lu", req->mem[i]->playerid);
		data_len = MAX_GLOBAL_SEND_BUF;
		int ret = sg_redis_client.hget_bin(server_key, key, (char *)conn_node_base::global_send_buf, &data_len);
		if (ret < 0)
		{
			continue;
		}
		PlayerRedisInfo *rplayer = player_redis_info__unpack(NULL, data_len, conn_node_base::global_send_buf);
		if (rplayer == NULL)
		{
			continue;
		}
		pack_team_mem_info(req->mem[i], rplayer);

		player_redis_info__free_unpacked(rplayer, NULL);
	}
	fast_send_msg(&conn_node_friendsrv::connecter, extern_data, head->msg_id, team_info__pack, *req);
	team_info__free_unpacked(req, NULL);


	LOG_DEBUG("%s: team info %lu len[%d] ret", __FUNCTION__, extern_data->player_id, data_len);
}

void conn_node_friendsrv::handle_team_apply_list()
{
	PROTO_HEAD *head = get_head();
	EXTERN_DATA *extern_data = get_extern_data(head);
	char key[128];
	int data_len = ENDION_FUNC_4(head->len) - sizeof(PROTO_HEAD)-sizeof(EXTERN_DATA);

	TeamApplyerList *req = team_applyer_list__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return;
	}
	for (size_t i = 0; i < req->n_apply; ++i)
	{
		if (req->apply[i]->online)
		{
			continue;
		}
		sprintf(key, "%lu", req->apply[i]->playerid);
		data_len = MAX_GLOBAL_SEND_BUF;
		int ret = sg_redis_client.hget_bin(server_key, key, (char *)conn_node_base::global_send_buf, &data_len);
		if (ret < 0)
		{
			continue;
		}
		PlayerRedisInfo *rplayer = player_redis_info__unpack(NULL, data_len, conn_node_base::global_send_buf);
		if (rplayer == NULL)
		{
			continue;
		}
		pack_team_mem_info(req->apply[i], rplayer);

		player_redis_info__free_unpacked(rplayer, NULL);
	}
	fast_send_msg(&conn_node_friendsrv::connecter, extern_data, head->msg_id, team_applyer_list__pack, *req);
	team_applyer_list__free_unpacked(req, NULL);


	LOG_DEBUG("%s: team info %lu len[%d] ret", __FUNCTION__, extern_data->player_id, data_len);
}

void conn_node_friendsrv::handle_team_list()
{
	PROTO_HEAD *head = get_head();
	EXTERN_DATA *extern_data = get_extern_data(head);
	char key[128];
	int data_len = ENDION_FUNC_4(head->len) - sizeof(PROTO_HEAD)-sizeof(EXTERN_DATA);

	TeamList *req = team_list__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return;
	}
	for (size_t t = 0; t < req->n_team; ++t)
	{
		TeamListInfo *pTeam = req->team[t];

		for (size_t i = 0; i < pTeam->n_lead; ++i)
		{
			if (pTeam->lead[i]->online)
			{
				continue;
			}
			sprintf(key, "%lu", pTeam->lead[i]->playerid);
			data_len = MAX_GLOBAL_SEND_BUF;
			int ret = sg_redis_client.hget_bin(server_key, key, (char *)conn_node_base::global_send_buf, &data_len);
			if (ret < 0)
			{
				continue;
			}
			PlayerRedisInfo *rplayer = player_redis_info__unpack(NULL, data_len, conn_node_base::global_send_buf);
			if (rplayer == NULL)
			{
				continue;
			}
			pack_team_mem_info(pTeam->lead[i], rplayer);

			player_redis_info__free_unpacked(rplayer, NULL);
		}
	}
	fast_send_msg(&conn_node_friendsrv::connecter, extern_data, head->msg_id, team_list__pack, *req);
	team_list__free_unpacked(req, NULL);

	LOG_DEBUG("%s: team info %lu len[%d] ret", __FUNCTION__, extern_data->player_id, data_len);
}

void conn_node_friendsrv::handle_zhenying_fight_myside_score()
{
	PROTO_HEAD *head = get_head();
	EXTERN_DATA *extern_data = get_extern_data(head);
	char key[128];
	int data_len = ENDION_FUNC_4(head->len) - sizeof(PROTO_HEAD)-sizeof(EXTERN_DATA);

	SideScore *req = side_score__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return;
	}
	for (size_t t = 0; t < req->n_side; ++t)
	{
		OneScore *pOne = req->side[t];
		if (!pOne->online)
		{
			sprintf(key, "%lu", pOne->playerid);
			data_len = MAX_GLOBAL_SEND_BUF;
			int ret = sg_redis_client.hget_bin(server_key, key, (char *)conn_node_base::global_send_buf, &data_len);
			if (ret < 0)
			{
				continue;
			}
			PlayerRedisInfo *rplayer = player_redis_info__unpack(NULL, data_len, conn_node_base::global_send_buf);
			if (rplayer == NULL)
			{
				continue;
			}
			strcpy(pOne->name, rplayer->name);

			player_redis_info__free_unpacked(rplayer, NULL);
		}
	}
	fast_send_msg(&conn_node_friendsrv::connecter, extern_data, head->msg_id, side_score__pack, *req);
	side_score__free_unpacked(req, NULL);

	LOG_DEBUG("%s: team info %lu len[%d] ret", __FUNCTION__, extern_data->player_id, data_len);
}
void conn_node_friendsrv::handle_zhenying_fight_settle()
{
	PROTO_HEAD *head = get_head();
	EXTERN_DATA *extern_data = get_extern_data(head);
	char key[128];
	int data_len = ENDION_FUNC_4(head->len) - sizeof(PROTO_HEAD)-sizeof(EXTERN_DATA);

	ZhenYingResult *req = zhen_ying_result__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return;
	}
	OneScore **oneArr[2];
	uint32_t oneNum[2];
	oneArr[0] = req->fulongguo;
	oneArr[1] = req->dianfenggu;
	oneNum[0] = req->n_fulongguo;
	oneNum[1] = req->n_dianfenggu;
	for (uint32_t i = 0; i < 2; ++i)
	{
		for (size_t t = 0; t < oneNum[i]; ++t)
		{
			OneScore *pOne = oneArr[i][t];
			if (!pOne->online)
			{
				sprintf(key, "%lu", pOne->playerid);
				data_len = MAX_GLOBAL_SEND_BUF;
				int ret = sg_redis_client.hget_bin(server_key, key, (char *)conn_node_base::global_send_buf, &data_len);
				if (ret < 0)
				{
					continue;
				}
				PlayerRedisInfo *rplayer = player_redis_info__unpack(NULL, data_len, conn_node_base::global_send_buf);
				if (rplayer == NULL)
				{
					continue;
				}
				strcpy(pOne->name, rplayer->name);

				player_redis_info__free_unpacked(rplayer, NULL);
			}
		}
	}

	fast_send_msg(&conn_node_friendsrv::connecter, extern_data, head->msg_id, zhen_ying_result__pack, *req);
	zhen_ying_result__free_unpacked(req, NULL);

	LOG_DEBUG("%s: team info %lu len[%d] ret", __FUNCTION__, extern_data->player_id, data_len);
}

void conn_node_friendsrv::handle_lately_chat()
{
	PROTO_HEAD *head = get_head();
	EXTERN_DATA *extern_data = get_extern_data(head);
	char key[128];
	int data_len = ENDION_FUNC_4(head->len) - sizeof(PROTO_HEAD)-sizeof(EXTERN_DATA);

	AnsLatelyChat *req = ans_lately_chat__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return;
	}

	for (uint32_t i = 0; i < req->n_player; ++i)
	{
		if (!req->player[i]->online)
		{
			sprintf(key, "%lu", req->player[i]->playerid);
			data_len = MAX_GLOBAL_SEND_BUF;
			int ret = sg_redis_client.hget_bin(server_key, key, (char *)conn_node_base::global_send_buf, &data_len);
			if (ret < 0)
			{
				continue;
			}
			PlayerRedisInfo *rplayer = player_redis_info__unpack(NULL, data_len, conn_node_base::global_send_buf);
			if (rplayer == NULL)
			{
				continue;
			}
			strcpy(req->player[i]->name, rplayer->name);
			req->player[i]->job = rplayer->job;
			req->player[i]->lv = rplayer->lv;
			req->player[i]->head = rplayer->head_icon;
			req->player[i]->zhenying = rplayer->zhenying;

			player_redis_info__free_unpacked(rplayer, NULL);
		}
	}

	fast_send_msg(&conn_node_friendsrv::connecter, extern_data, head->msg_id, ans_lately_chat__pack, *req);
	ans_lately_chat__free_unpacked(req, NULL);

}

void conn_node_friendsrv::handle_chengjie_list()
{
	PROTO_HEAD *head = get_head();
	EXTERN_DATA *extern_data = get_extern_data(head);
	char key[128];
	int data_len = ENDION_FUNC_4(head->len) - sizeof(PROTO_HEAD)-sizeof(EXTERN_DATA);

	ChengjieList *req = chengjie_list__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return;
	}
	for (size_t t = 0; t < req->n_task; ++t)
	{
		ChengjieTask *pTask = req->task[t];


		if (pTask->online)
		{
			continue;
		}
		sprintf(key, "%lu", pTask->playerid);
		data_len = MAX_GLOBAL_SEND_BUF;
		int ret = sg_redis_client.hget_bin(server_key, key, (char *)conn_node_base::global_send_buf, &data_len);
		if (ret < 0)
		{
			continue;
		}
		PlayerRedisInfo *rplayer = player_redis_info__unpack(NULL, data_len, conn_node_base::global_send_buf);
		if (rplayer == NULL)
		{
			continue;
		}

		pTask->icon = rplayer->head_icon;
		strcpy(pTask->name, rplayer->name);
		pTask->lv = rplayer->lv;
		pTask->job = rplayer->job;
		pTask->zhenying = rplayer->zhenying;
		pTask->fight = rplayer->fighting_capacity;

		player_redis_info__free_unpacked(rplayer, NULL);
	}
	for (size_t t = 0; t < req->n_task_myself; ++t)
	{
		ChengjieTask *pTask = req->task_myself[t];


		if (pTask->online)
		{
			continue;
		}
		sprintf(key, "%lu", pTask->playerid);
		data_len = MAX_GLOBAL_SEND_BUF;
		int ret = sg_redis_client.hget_bin(server_key, key, (char *)conn_node_base::global_send_buf, &data_len);
		if (ret < 0)
		{
			continue;
		}
		PlayerRedisInfo *rplayer = player_redis_info__unpack(NULL, data_len, conn_node_base::global_send_buf);
		if (rplayer == NULL)
		{
			continue;
		}

		pTask->icon = rplayer->head_icon;
		strcpy(pTask->name, rplayer->name);
		pTask->lv = rplayer->lv;
		pTask->job = rplayer->job;
		pTask->zhenying = rplayer->zhenying;
		pTask->fight = rplayer->fighting_capacity;

		player_redis_info__free_unpacked(rplayer, NULL);
	}
	fast_send_msg(&conn_node_friendsrv::connecter, extern_data, head->msg_id, chengjie_list__pack, *req);
	chengjie_list__free_unpacked(req, NULL);

	LOG_DEBUG("%s: team info %lu len[%d] ret", __FUNCTION__, extern_data->player_id, data_len);
}

void conn_node_friendsrv::handle_chengjie_task()
{
	PROTO_HEAD *head = get_head();
	EXTERN_DATA *extern_data = get_extern_data(head);
	char key[128];
	int data_len = ENDION_FUNC_4(head->len) - sizeof(PROTO_HEAD)-sizeof(EXTERN_DATA);

	ChengjieTask *pTask = chengjie_task__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!pTask)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return;
	}

	sprintf(key, "%lu", pTask->playerid);
	data_len = MAX_GLOBAL_SEND_BUF;
	int ret = sg_redis_client.hget_bin(server_key, key, (char *)conn_node_base::global_send_buf, &data_len);
	if (ret >= 0)
	{
		PlayerRedisInfo *rplayer = player_redis_info__unpack(NULL, data_len, conn_node_base::global_send_buf);
		if (rplayer != NULL)
		{
			pTask->icon = rplayer->head_icon;
			strcpy(pTask->name, rplayer->name);
			pTask->lv = rplayer->lv;
			pTask->job = rplayer->job;
			pTask->fight = rplayer->fighting_capacity;

			player_redis_info__free_unpacked(rplayer, NULL);
		}
	}

	fast_send_msg(&conn_node_friendsrv::connecter, extern_data, head->msg_id, chengjie_task__pack, *pTask);
	chengjie_task__free_unpacked(pTask, NULL);

	LOG_DEBUG("%s: team info %lu len[%d] ret", __FUNCTION__, extern_data->player_id, data_len);
}

void send_system_msg(char * str, uint64_t pid)
{
	Chat send;
	chat__init(&send);
	send.contain = str;
	send.channel = CHANNEL__system;
	send.sendname = NULL;
	send.sendplayerid = 0;
	send.sendplayerlv = 0;
	send.sendplayerjob = 0;
	//if (pid == 0)
	//{
	//	conn_node_gamesrv::send_to_all_player(MSG_ID_CHAT_NOTIFY, (void *)&send, (pack_func)chat__pack);
	//}
	//else
	{
		EXTERN_DATA extern_data;
		extern_data.player_id = pid;
		fast_send_msg(&conn_node_friendsrv::connecter, &extern_data, MSG_ID_CHAT_NOTIFY, chat__pack, send);
	}
}
void conn_node_friendsrv::handle_chengjie_task_complete()
{
	PROTO_HEAD *head = get_head();
	EXTERN_DATA *extern_data = get_extern_data(head);
	char key[128];
	int data_len = ENDION_FUNC_4(head->len) - sizeof(PROTO_HEAD)-sizeof(EXTERN_DATA);

	ChengjieTaskComplete *pTask = chengjie_task_complete__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!pTask)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return;
	}

	PlayerRedisInfo *pInvestor = NULL;
	PlayerRedisInfo *pTarget = NULL;
	PlayerRedisInfo *pAcceptor = NULL;

	sprintf(key, "%lu", pTask->investor);
	data_len = MAX_GLOBAL_SEND_BUF;
	int ret = sg_redis_client.hget_bin(server_key, key, (char *)conn_node_base::global_send_buf, &data_len);
	if (ret >= 0)
	{
		pInvestor = player_redis_info__unpack(NULL, data_len, conn_node_base::global_send_buf);
	}
	sprintf(key, "%lu", pTask->target);
	data_len = MAX_GLOBAL_SEND_BUF;
	ret = sg_redis_client.hget_bin(server_key, key, (char *)conn_node_base::global_send_buf, &data_len);
	if (ret >= 0)
	{
		pTarget = player_redis_info__unpack(NULL, data_len, conn_node_base::global_send_buf);
	}
	sprintf(key, "%lu", pTask->acceptor);
	data_len = MAX_GLOBAL_SEND_BUF;
	ret = sg_redis_client.hget_bin(server_key, key, (char *)conn_node_base::global_send_buf, &data_len);
	if (ret >= 0)
	{
		pAcceptor = player_redis_info__unpack(NULL, data_len, conn_node_base::global_send_buf);
	}

	Chat notify;
	chat__init(&notify);
	char str[1024 * 2];
	char name[MAX_PLAYER_NAME_LEN] = "神秘人";
	if (!pTask->anonymous)
	{
		if (pInvestor != NULL)
		{
			strncpy(name, pInvestor->name, MAX_PLAYER_NAME_LEN);

			notify.sendplayerid = pTask->investor;
			notify.sendplayerlv = pInvestor->lv;
			notify.sendplayerjob = pInvestor->job;
		}
	}
	ParameterTable * configSpeek = get_config_by_id(161000093, &parameter_config);
	sprintf(str, configSpeek->parameter2, name, pTask->declaration);
	send_system_msg(str, pTask->target);

	configSpeek = get_config_by_id(161000092, &parameter_config);
	if (pTarget != NULL)
	{
		sprintf(str, configSpeek->parameter2, name, pTarget->name, pTask->declaration);
	}

	notify.contain = str;
	notify.channel = CHANNEL__system;
	if (pTask->step < 3)
	{
		notify.sendname = NULL;
		notify.sendplayerid = 0;
		send_to_all_player(MSG_ID_CHAT_NOTIFY, (void *)&notify, (pack_func)chat__pack);
	}
	else
	{
		ChatHorse send;
		chat_horse__init(&send);
		send.id = 0;
		send.prior = 1;
		send.content = str;
		send_to_all_player(MSG_ID_CHAT_HORSE_NOTIFY, &send, (pack_func)chat_horse__pack);
	}

	if (pAcceptor != NULL && pTarget != NULL)
	{
		char strContain[512] = "XXX不负所托，在4月1日在YYY完成了对ZZZ的暗杀";
		char title[128] = "悬赏已完成";
		sprintf(strContain, "%s不负所托，在%d月%d日在%s完成了对%s的暗杀", pAcceptor->name, time_helper::get_cur_month_by_year(0), time_helper::get_cur_day_by_month(0),
			pTask->scene, pTarget->name);
		::send_mail(&conn_node_friendsrv::connecter, pTask->investor, 0, title, NULL, strContain, NULL, NULL, MAGIC_TYPE_YAOSHI);
	}

	if (pInvestor != NULL)
	{
		player_redis_info__free_unpacked(pInvestor, NULL);
	}
	if (pTarget != NULL)
	{
		player_redis_info__free_unpacked(pTarget, NULL);
	}
	if (pAcceptor != NULL)
	{
		player_redis_info__free_unpacked(pAcceptor, NULL);
	}

	chengjie_task_complete__free_unpacked(pTask, NULL);

	LOG_DEBUG("%s: team info %lu len[%d] ret", __FUNCTION__, extern_data->player_id, data_len);
}

void conn_node_friendsrv::handle_add_wanyaoka()
{
	ListWanyaokaAnswer *answer = NULL;
	ListWanyaokaAnswer new_answer;
	uint32_t wanyaoka[MAX_WANYAOKA_PER_PLAYER];
	//	PROTO_ADD_WANYAOKA *req = (PROTO_ADD_WANYAOKA *)buf_head();
	PROTO_ADD_WANYAOKA *req = (PROTO_ADD_WANYAOKA *)((PROTO_HEAD *)buf_head())->data;
	char key[128];
	sprintf(key, "%lu", req->player_id);
	int data_len = MAX_GLOBAL_SEND_BUF;
	int ret = sg_redis_client.hget_bin(server_wyk_key, key, (char *)conn_node_base::global_send_buf, &data_len);
	if (ret == 0)
	{
		//		answer = list_wanyaoka_answer__unpack(NULL, get_data_len(), (uint8_t *)get_data());
		answer = list_wanyaoka_answer__unpack(NULL, data_len, conn_node_base::global_send_buf);
	}
	list_wanyaoka_answer__init(&new_answer);
	new_answer.wanyaoka_id = wanyaoka;
	if (answer)
	{
		int n_wanyaoka = 0;
		for (int i = 0; i < MAX_WANYAOKA_EACH_TIME; ++i)
		{
			if (req->wanyaoka[i] == 0)
				continue;
			for (uint32_t j = 0; j < answer->n_wanyaoka_id; ++j)
			{
				if (req->wanyaoka[i] == answer->wanyaoka_id[j])
				{
					req->wanyaoka[i] = 0;
					break;
				}
			}
		}
		for (int i = 0; i < MAX_WANYAOKA_EACH_TIME; ++i)
		{
			if (req->wanyaoka[i] != 0)
				++n_wanyaoka;
		}
		if (n_wanyaoka == 0)
		{
			list_wanyaoka_answer__free_unpacked(answer, NULL);
			return;
		}

		for (size_t i = 0; i < answer->n_wanyaoka_id; ++i)
		{
			wanyaoka[i] = answer->wanyaoka_id[i];
		}
		new_answer.n_wanyaoka_id = answer->n_wanyaoka_id;
		list_wanyaoka_answer__free_unpacked(answer, NULL);
	}

	for (int i = 0; i < MAX_WANYAOKA_EACH_TIME; ++i)
	{
		if (req->wanyaoka[i] == 0)
			continue;
		wanyaoka[new_answer.n_wanyaoka_id++] = req->wanyaoka[i];
	}
	size_t size = list_wanyaoka_answer__pack(&new_answer, conn_node_base::global_send_buf);
	ret = sg_redis_client.hset_bin(server_wyk_key, key, (const char *)conn_node_base::global_send_buf, size);
	if (ret != 1 && ret != 0)
	{
		LOG_ERR("%s: player[%lu] oper failed, ret = %d", __FUNCTION__, req->player_id, ret);
	}
	LOG_DEBUG("%s: save %lu len[%lu] ret = %d", __FUNCTION__, req->player_id, size, ret);
}

void conn_node_friendsrv::handle_chengjie_money_back()
{
	PROTO_HEAD *head = get_head();
	EXTERN_DATA *extern_data = get_extern_data(head);
	char key[128];
	int data_len = ENDION_FUNC_4(head->len) - sizeof(PROTO_HEAD)-sizeof(EXTERN_DATA);

	ChengjieMoney *req = chengjie_money__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return;
	}

	sprintf(key, "%lu", req->target);
	data_len = MAX_GLOBAL_SEND_BUF;
	int ret = sg_redis_client.hget_bin(server_key, key, (char *)conn_node_base::global_send_buf, &data_len);
	if (ret >= 0)
	{
		PlayerRedisInfo *rplayer = player_redis_info__unpack(NULL, data_len, conn_node_base::global_send_buf);
		if (rplayer != NULL)
		{
			ParameterTable * config = get_config_by_id(161000096, &parameter_config);
			if (config != NULL)
			{
				uint64_t befor = req->cd - config->parameter1[0];
				std::map<uint32_t, uint32_t> attachs;
				attachs[201010001] = req->money;
				char strContain[512] = "您在XX月XX日发布对YY玩家的悬赏令已过期，现退回悬赏金额";
				char title[128] = "悬赏已过期，退回金额";
				sprintf(strContain, "您在%d月%d日发布对%s玩家的悬赏令已过期，现退回悬赏金额", time_helper::get_cur_month_by_year(befor), time_helper::get_cur_day_by_month(befor), rplayer->name);
				send_mail(&conn_node_friendsrv::connecter, req->pid, 0, title, NULL, strContain, NULL, &attachs, MAGIC_TYPE_YAOSHI);
			}
			player_redis_info__free_unpacked(rplayer, NULL);
		}
	}

	chengjie_money__free_unpacked(req, NULL);

	LOG_DEBUG("%s: team info %lu len[%d] ret", __FUNCTION__, extern_data->player_id, data_len);;
}

void conn_node_friendsrv::handle_save_wanyaoka()
{
	PROTO_HEAD *req = get_head();
	EXTERN_DATA *extern_data = get_extern_data(req);
	char key[128];
	sprintf(key, "%lu", extern_data->player_id);
	int data_len = ENDION_FUNC_4(req->len) - sizeof(PROTO_HEAD)-sizeof(EXTERN_DATA);
	int ret = sg_redis_client.hset_bin(server_wyk_key, key, (const char *)&req->data[0], data_len);
	if (ret < 0)
	{
		LOG_ERR("%s: %lu oper failed, ret = %d", __FUNCTION__, extern_data->player_id, ret);
	}
	LOG_DEBUG("%s: save %lu len[%d] ret = %d", __FUNCTION__, extern_data->player_id, data_len, ret);
}
void conn_node_friendsrv::handle_list_wanyaoka()
{
	PROTO_HEAD *send_head = (PROTO_HEAD *)global_send_buf;
	PROTO_HEAD *req = get_head();
	EXTERN_DATA *extern_data = get_extern_data(req);
	char key[128];
	sprintf(key, "%lu", extern_data->player_id);
	int data_len = MAX_GLOBAL_SEND_BUF;
	int ret = sg_redis_client.hget_bin(server_wyk_key, key, (char *)send_head->data, &data_len);
	if (ret < 0)
	{
		ListWanyaokaAnswer answer;
		list_wanyaoka_answer__init(&answer);
		fast_send_msg(&conn_node_friendsrv::connecter, extern_data, MSG_ID_LIST_WANYAOKA_ANSWER, list_wanyaoka_answer__pack, answer);
		return;
	}
	fast_send_msg_base(&conn_node_friendsrv::connecter, extern_data, MSG_ID_LIST_WANYAOKA_ANSWER, data_len, req->seq);
}

void conn_node_friendsrv::handle_tower_max()
{
	PROTO_HEAD *head = get_head();
	EXTERN_DATA *extern_data = get_extern_data(head);
	char key[128];
	int data_len = ENDION_FUNC_4(head->len) - sizeof(PROTO_HEAD)-sizeof(EXTERN_DATA);

	ReqTowerMax *req = req_tower_max__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		return;
	}
	
	TowerMax send;
	tower_max__init(&send);
	send.cur_lv = req->lv;

	data_len = MAX_GLOBAL_SEND_BUF;
	TowerRecord *tMax = NULL;
	int ret = sg_redis_client.hget_bin(tower_max_key, server_key, (char *)conn_node_base::global_send_buf, &data_len);
	if (ret >= 0)
	{
		tMax = tower_record__unpack(NULL, data_len, conn_node_base::global_send_buf); 
		if (tMax != NULL && req->lv < tMax->n_maxcd)
		{
			send.maxcd = tMax->maxcd[req->lv]->cd;
			send.cd_name = tMax->maxcd[req->lv]->name;
			send.maxlv = tMax->maxlv;
			send.lv_name = tMax->lv_name;
		}
	}
	
	sprintf(key, "%lu", extern_data->player_id);
	data_len = MAX_GLOBAL_SEND_BUF;
	TowerCd *tCd = NULL;
	ret = sg_redis_client.hget_bin(tower_cd_key, key, (char *)conn_node_base::global_send_buf, &data_len);
	if (ret >= 0)
	{
		tCd = tower_cd__unpack(NULL, data_len, conn_node_base::global_send_buf);
		if (tCd != NULL && tCd->cd != NULL && req->lv < tCd->n_cd)
		{
			send.self_cd = tCd->cd[req->lv];
		}
	}
	
	fast_send_msg(&connecter, extern_data, MSG_ID_TOWER_MAX_ANSWER, tower_max__pack, send);
	req_tower_max__free_unpacked(req, NULL);
	if (tMax != NULL)
	{
		tower_record__free_unpacked(tMax, NULL);
	}
	if (tCd != NULL)
	{
		tower_cd__free_unpacked(tCd, NULL);
	}

	LOG_DEBUG("%s: team info %lu len[%d] ret", __FUNCTION__, extern_data->player_id, data_len);
}

void conn_node_friendsrv::handle_update_tower()
{
	PROTO_HEAD *head = get_head();
	EXTERN_DATA *extern_data = get_extern_data(head);
	char key[128];
	int data_len = ENDION_FUNC_4(head->len) - sizeof(PROTO_HEAD)-sizeof(EXTERN_DATA);

	sprintf(key, "%lu", extern_data->player_id);
	data_len = MAX_GLOBAL_SEND_BUF;
	int ret = sg_redis_client.hget_bin(server_key, key, (char *)conn_node_base::global_send_buf, &data_len);
	if (ret < 0)
	{
		return;
	}
	PlayerRedisInfo *rplayer = player_redis_info__unpack(NULL, data_len, conn_node_base::global_send_buf);
	if (rplayer == NULL)
	{
		return;
	}

	UpdateTower *req = update_tower__unpack(NULL, get_data_len(), (uint8_t *)get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] can not unpack player[%lu] cmd", __FUNCTION__, __LINE__, extern_data->player_id);
		player_redis_info__free_unpacked(rplayer, NULL);
		return;
	}

	static const int MAX_TOWER_LEVEL_SAVE = 100;

	data_len = MAX_GLOBAL_SEND_BUF;
	bool beNew = false;
	bool save = false;
	TowerRecord saveMax;
	tower_record__init(&saveMax);
	TowerRecord *tMax = NULL;
	TowerMaxCd arr[MAX_TOWER_LEVEL_SAVE + 1];
	TowerMaxCd *arrP[MAX_TOWER_LEVEL_SAVE + 1];
	ret = sg_redis_client.hget_bin(tower_max_key, server_key, (char *)conn_node_base::global_send_buf, &data_len);
	if (ret < 0)
	{
		beNew = true;
	}
	else
	{
		tMax = tower_record__unpack(NULL, data_len, conn_node_base::global_send_buf);
		if (tMax == NULL)
		{
			beNew = true;
		}
		else
		{
			saveMax.maxcd = tMax->maxcd;
			saveMax.n_maxcd = tMax->n_maxcd;
			saveMax.maxlv = tMax->maxlv;
			saveMax.lv_name = tMax->lv_name;
			if (tMax->maxlv <= req->lv)
			{
				saveMax.maxlv = req->lv;
				saveMax.lv_name = rplayer->name;
				save = true;
			}
			if (tMax->maxcd[req->lv]->cd == 0 || tMax->maxcd[req->lv]->cd > req->cd)
			{
				for (uint32_t i = 0; i <= MAX_TOWER_LEVEL_SAVE; ++i)
				{
					tower_max_cd__init(&arr[i]);
					arrP[i] = &arr[i];
					arrP[i]->name = tMax->maxcd[i]->name;
					arrP[i]->cd = tMax->maxcd[i]->cd;
				}
				saveMax.maxcd = arrP;
				saveMax.n_maxcd = 100 + 1;
				saveMax.maxcd[req->lv]->cd = req->cd;
				saveMax.maxcd[req->lv]->name = rplayer->name;
				save = true;
			}
		}
	}
	if (beNew)
	{
		for (uint32_t i = 0; i <= MAX_TOWER_LEVEL_SAVE; ++i)
		{
			tower_max_cd__init(&arr[i]);
			arrP[i] = &arr[i];
		}
		save = true;
		saveMax.maxcd = arrP;
		saveMax.n_maxcd = MAX_TOWER_LEVEL_SAVE + 1;
		saveMax.maxlv = req->lv;
		saveMax.lv_name = rplayer->name;
		saveMax.maxcd[req->lv]->cd = req->cd;
		saveMax.maxcd[req->lv]->name = rplayer->name;
	}
	if (save)
	{
		data_len = tower_record__pack(&saveMax, (uint8_t *)conn_node_base::global_send_buf);
		ret = sg_redis_client.hset_bin(tower_max_key, server_key, (char *)conn_node_base::global_send_buf, data_len);
		if (ret < 0)
		{
			LOG_ERR("%s: save tower max fail, ret = %d", __FUNCTION__, ret);
		}
	}
	if (tMax != NULL)
	{
		tower_record__free_unpacked(tMax, NULL);
	}

	data_len = MAX_GLOBAL_SEND_BUF;
	uint32_t arrCd[MAX_TOWER_LEVEL_SAVE + 1];
	beNew = false;
	save = false;
	TowerCd saveCd;
	tower_cd__init(&saveCd);
	TowerCd *tCd = NULL;
	ret = sg_redis_client.hget_bin(tower_cd_key, key, (char *)conn_node_base::global_send_buf, &data_len);
	if (ret < 0)
	{
		
		beNew = true;
	}
	else
	{
		tCd = tower_cd__unpack(NULL, data_len, conn_node_base::global_send_buf);
		if (tCd == NULL || tCd->cd == NULL)
		{
			beNew = true;
		}
		else
		{
			if (tCd->cd[req->lv] == 0 || tCd->cd[req->lv] > req->cd)
			{
				tCd->cd[req->lv] = req->cd;
				saveCd.cd = tCd->cd;
				saveCd.n_cd = tCd->n_cd;
				save = true;
			}
		}
		
	}
	if (beNew)
	{
		saveCd.cd = arrCd;
		saveCd.n_cd = MAX_TOWER_LEVEL_SAVE + 1;
		memset(arrCd, 0, sizeof(arrCd));
		arrCd[req->lv] = req->cd;
		save = true;
	}
	if (save)
	{
		data_len = tower_cd__pack(&saveCd, (uint8_t *)conn_node_base::global_send_buf);
		ret = sg_redis_client.hset_bin(tower_cd_key, key, (char *)conn_node_base::global_send_buf, data_len);
		if (ret < 0)
		{
			LOG_ERR("%s: save tower max fail, ret = %d", __FUNCTION__, ret);
		}
	}
	if (tCd != NULL)
	{
		tower_cd__free_unpacked(tCd, NULL);
	}

	update_tower__free_unpacked(req, NULL);
	player_redis_info__free_unpacked(rplayer, NULL);


	LOG_DEBUG("%s: team info %lu len[%d] ret", __FUNCTION__, extern_data->player_id, data_len);
}

int conn_node_friendsrv::recv_func(evutil_socket_t fd)
{
	//	__attribute__((unused)) EXTERN_DATA *extern_data;
	//	__attribute__((unused)) PROTO_HEAD *head;	
	for (;;) {
		int ret = get_one_buf();
		if (ret == 0) {

			//			head = (PROTO_HEAD *)buf_head();
			//			extern_data = get_extern_data(head);

			uint64_t times = time_helper::get_micro_time();
			time_helper::set_cached_time(times / 1000);

			int cmd = get_cmd();
			switch (cmd)
			{
			case MSG_ID_LIST_WANYAOKA_REQUEST:
				handle_list_wanyaoka();
				break;
			case MSG_ID_TEAM_INFO_NOTIFY:
				handle_team_info();
				break;
			case MSG_ID_ZHENYING_FIGHT_MYSIDE_SCORE_NOTIFY:
				handle_zhenying_fight_myside_score();
				break;
			case MSG_ID_ZHENYING_FIGHT_SETTLE_NOTIFY:
				handle_zhenying_fight_settle();
				break;
			case MSG_ID_LATELY_CHAT_ANSWER:
				handle_lately_chat();
				break;
			case MSG_ID_APPLYERLIST_TEAM_NOTIFY:
				handle_team_apply_list();
				break;
			case MSG_ID_TEAM_LIST_ANSWER:
				handle_team_list();
				break;
			case SERVER_PROTO_ADD_WANYAOKA:
				handle_add_wanyaoka();
				break;
			case SERVER_PROTO_SAVE_WANYAOKA:
				handle_save_wanyaoka();
				break;
			case SERVER_PROTO_UPDATE_TOWER_REQUEST:
				handle_update_tower();
				break;
			case MSG_ID_TOWER_MAX_REQUEST:
				handle_tower_max();
				break;
			case SERVER_PROTO_LIST_WANYAOKA:
				handle_list_wanyaoka();
				break;
			case SERVER_PROTO_GET_OFFLINE_CACHE_REQUEST:
				handle_cache_get();
				break;
			case SERVER_PROTO_CLEAR_OFFLINE_CACHE:
				handle_cache_clear();
				break;
			case SERVER_PROTO_INSERT_OFFLINE_CACHE:
				handle_cache_insert();
				break;
			case MSG_ID_REFRESH_CHENGJIE_LIST_ANSWER:
				handle_chengjie_list();
				break;
			case SERVER_PROTO_CHENGJIE_MONEY_BACK:
				handle_chengjie_money_back();
				break;
			case MSG_ID_CUR_CHENGJIE_TASK_NOTIFY:
				handle_chengjie_task();
				break;
			case SERVER_PROTO_CHENGJIE_TASK_COMPLETE_REQUEST:
				handle_chengjie_task_complete();
				break;
			case MSG_ID_CHOSE_ZHENYING_REQUEST:
				handle_chose_zhenying();
				break;
			case MSG_ID_CHANGE_ZHENYING_REQUEST:
				handle_change_zhenying();
				break;
			case MSG_ID_ZHENYING_POWER_REQUEST:
				handle_zhenying_power();
				break;
			case SERVER_PROTO_ZHENYING_CHANGE_POWER_REQUEST:
				handle_zhenying_change_power();
				break;
			case SERVER_PROTO_ADD_ZHENYING_KILL_REQUEST:
				//handle_zhenying_add_kill();
				break;
			case MSG_ID_FRIEND_INFO_REQUEST:
				handle_friend_info_request();
				break;
			case MSG_ID_FRIEND_ADD_CONTACT_REQUEST:
				handle_friend_add_contact_request();
				break;
			case MSG_ID_FRIEND_DEL_CONTACT_REQUEST:
				handle_friend_del_contact_request();
				break;
			case MSG_ID_FRIEND_ADD_BLOCK_REQUEST:
				handle_friend_add_block_request();
				break;
			case MSG_ID_FRIEND_DEL_BLOCK_REQUEST:
				handle_friend_del_block_request();
				break;
			case MSG_ID_FRIEND_DEL_ENEMY_REQUEST:
				handle_friend_del_enemy_request();
				break;
			case MSG_ID_FRIEND_CREATE_GROUP_REQUEST:
				handle_friend_create_group_request();
				break;
			case MSG_ID_FRIEND_EDIT_GROUP_REQUEST:
				handle_friend_edit_group_request();
				break;
			case MSG_ID_FRIEND_REMOVE_GROUP_REQUEST:
				handle_friend_remove_group_request();
				break;
			case MSG_ID_FRIEND_MOVE_PLAYER_GROUP_REQUEST:
				handle_friend_move_player_group_request();
				break;
			case MSG_ID_FRIEND_DEAL_APPLY_REQUEST:
				handle_friend_deal_apply_request();
				break;
			case SERVER_PROTO_FRIEND_RECOMMEND:
				handle_friend_recommend_request();
				break;
			case SERVER_PROTO_FRIEND_CHAT:
				handle_friend_chat_request();
				break;
			case SERVER_PROTO_PLAYER_ONLINE_NOTIFY:
				handle_player_online_notify();
				break;
			case SERVER_PROTO_KICK_ROLE_NOTIFY:
				handle_player_offline_notify();
				break;
			case SERVER_PROTO_FRIEND_ADD_ENEMY:
				handle_friend_add_enemy_request();
				break;
			case MSG_ID_FRIEND_SEND_GIFT_REQUEST:
				handle_friend_send_gift_request();
				break;
			case SERVER_PROTO_FRIEND_GIFT_COST_ANSWER:
				handle_friend_gift_cost_answer();
				break;
			case SERVER_PROTO_FRIENDSRV_COST_ANSWER:
				handle_friend_cost_answer();
				break;
			case SERVER_PROTO_FRIEND_TURN_SWITCH:
				handle_friend_turn_switch_request();
				break;
			case SERVER_PROTO_FRIEND_EXTEND_CONTACT_REQUEST:
				handle_friend_extend_contact_request();
				break;
			case SERVER_PROTO_FRIEND_SYNC_RENAME:
				handle_friend_rename_request();
				break;
			case MSG_ID_FRIEND_TRACK_ENEMY_REQUEST:
				handle_friend_track_enemy_request();
				break;
			case SERVER_PROTO_FRIEND_TRACK_ENEMY_ANSWER:
				handle_friend_track_enemy_answer();
				break;
			case MSG_ID_FRIEND_AUTO_ACCEPT_APPLY_REQUEST:
				handle_friend_auto_accept_apply_request();
				break;
			case MSG_ID_FRIEND_ID_OR_NO_EACH_OTHER_REQUEST:
				handle_friend_is_or_no_each_other_request(); 
			case SERVER_PROTO_FRIEND_DUMP_CLOSENESS_REQUEST:
				handle_friend_clean_closeness_request();
			}

		}

		if (ret < 0) {
			LOG_INFO("%s: connect closed from fd %u, err = %d", __FUNCTION__, fd, errno);
			exit(0);
			return (-1);
		}
		else if (ret > 0) {
			break;
		}

		ret = remove_one_buf();
	}
	return (0);
}

void cb_on_timer(evutil_socket_t, short, void *arg) {
	add_timer(sg_timeout, &sg_event_timer, NULL);
}

void conn_node_friendsrv::send_system_notice(uint64_t player_id, uint32_t id, std::vector<char*> *args, uint64_t target_id)
{
	SystemNoticeNotify nty;
	system_notice_notify__init(&nty);

	nty.id = id;
	if (args)
	{
		nty.n_args = args->size();
		nty.args = &((*args)[0]);
	}
	if (target_id > 0)
	{
		nty.targetid = target_id;
		nty.has_targetid = true;
	}

	EXTERN_DATA ext_data;
	ext_data.player_id = player_id;

	fast_send_msg(&connecter, &ext_data, MSG_ID_SYSTEM_NOTICE_NOTIFY, system_notice_notify__pack, nty);
}

int conn_node_friendsrv::broadcast_message(uint16_t msg_id, void *msg_data, pack_func packer, std::vector<uint64_t> &players)
{
	PROTO_HEAD_CONN_BROADCAST *head;
	PROTO_HEAD *real_head;

	head = (PROTO_HEAD_CONN_BROADCAST *)conn_node_base::global_send_buf;
	head->msg_id = ENDION_FUNC_2(SERVER_PROTO_BROADCAST);
	real_head = &head->proto_head;

	real_head->msg_id = ENDION_FUNC_2(msg_id);
	real_head->seq = 0;
	//	memcpy(real_head->data, msg_data, len);
	size_t len = 0;
	if (msg_data && packer)
	{
		len = packer(msg_data, (uint8_t *)real_head->data);
	}
	real_head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD)+len);

	uint64_t *ppp = (uint64_t*)((char *)&head->player_id + len);
	head->num_player_id = 0;
	for (std::vector<uint64_t>::iterator iter = players.begin(); iter != players.end(); ++iter)
	{
		ppp[head->num_player_id++] = *iter;
	}
	head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD_CONN_BROADCAST)+len + sizeof(uint64_t)* head->num_player_id);
	if (conn_node_friendsrv::connecter.send_one_msg((PROTO_HEAD *)head, 1) != (int)(ENDION_FUNC_4(head->len)))
	{
		LOG_ERR("[%s:%d] send to conn_srv failed err[%d]", __FUNCTION__, __LINE__, errno);
	}
	return 0;
}

void conn_node_friendsrv::handle_cache_get()
{
	PROTO_HEAD *req = get_head();
	EXTERN_DATA *extern_data = get_extern_data(req);

	char key[128];
	sprintf(key, "%lu", extern_data->player_id);

	PROTO_HEAD *send_head = (PROTO_HEAD *)global_send_buf;
	int data_len = MAX_GLOBAL_SEND_BUF;
	int ret = sg_redis_client.hget_bin(sg_player_cache_key, key, (char *)send_head->data, &data_len);
	if (ret < 0)
	{
		PlayerOfflineCache answer;
		player_offline_cache__init(&answer);
		fast_send_msg(&conn_node_friendsrv::connecter, extern_data, SERVER_PROTO_GET_OFFLINE_CACHE_ANSWER, player_offline_cache__pack, answer);
		return;
	}
	fast_send_msg_base(&conn_node_friendsrv::connecter, extern_data, SERVER_PROTO_GET_OFFLINE_CACHE_ANSWER, data_len, req->seq);
}

void conn_node_friendsrv::handle_cache_clear()
{
	PROTO_HEAD *req = get_head();
	EXTERN_DATA *extern_data = get_extern_data(req);

	char key[128];
	sprintf(key, "%lu", extern_data->player_id);

	if (sg_redis_client.exist(sg_player_cache_key, key) == 1)
	{
		int ret = sg_redis_client.hdel(sg_player_cache_key, key);
		if (ret < 0)
		{
			LOG_ERR("[%s,%d]: oper failed, ret = %d", __FUNCTION__, __LINE__, ret);
		}
	}
	LOG_DEBUG("%s: clear %lu", __FUNCTION__, extern_data->player_id);
}

static void save_player_cache(uint64_t player_id, char *buffer, int buffer_len)
{
	char key[128];
	sprintf(key, "%lu", player_id);

	int ret = sg_redis_client.hset_bin(sg_player_cache_key, key, buffer, buffer_len);
	if (ret < 0)
	{
		LOG_ERR("[%s,%d]: oper failed, ret = %d", __FUNCTION__, __LINE__, ret);
	}
	LOG_DEBUG("%s: save %lu ret = %d", __FUNCTION__, player_id, ret);
}

void conn_node_friendsrv::handle_cache_insert()
{
	PROTO_PLAYER_CACHE_INSERT *proto = (PROTO_PLAYER_CACHE_INSERT*)buf_head();

	char key[128];
	sprintf(key, "%lu", proto->player_id);

	char cache_data[1024 * 1024];
	int data_len = 1024 * 1024 - 1;
	int ret = sg_redis_client.hget_bin(sg_player_cache_key, key, (char *)cache_data, &data_len);
	PlayerOfflineCache *cache_info = NULL;
	if (ret < 0)
	{
		cache_info = (PlayerOfflineCache*)malloc(sizeof(PlayerOfflineCache));
		player_offline_cache__init(cache_info);
	}
	else
	{
		cache_info = player_offline_cache__unpack(NULL, data_len, (uint8_t *)cache_data);
		if (!cache_info)
		{
			LOG_ERR("[%s:%d] player [%lu] unpack db cache failed", __FUNCTION__, __LINE__, proto->player_id);
			return;
		}
	}

	switch (proto->type)
	{
	case CACHE_SUB_EXP:
	{
						  int msg_len = get_len() - sizeof(PROTO_PLAYER_CACHE_INSERT);
						  CacheSubExp *sub_exp = cache_sub_exp__unpack(NULL, msg_len, proto->data);
						  if (!sub_exp)
						  {
							  LOG_ERR("[%s:%d] player [%lu] unpack req failed", __FUNCTION__, __LINE__, proto->player_id);
							  player_offline_cache__free_unpacked(cache_info, NULL);
							  return;
						  }

						  cache_info->n_sub_exps++;
						  cache_info->sub_exps = (CacheSubExp**)realloc(cache_info->sub_exps, cache_info->n_sub_exps * sizeof(CacheSubExp));

						  cache_info->sub_exps[cache_info->n_sub_exps - 1] = sub_exp;

						  data_len = player_offline_cache__pack(cache_info, (uint8_t*)cache_data);
						  save_player_cache(proto->player_id, cache_data, data_len);

						  cache_info->n_sub_exps--;
						  cache_sub_exp__free_unpacked(sub_exp, NULL);
	}
		break;
	case CACHE_PVP_RAID_LOSE:
	{
								int msg_len = get_len() - sizeof(PROTO_PLAYER_CACHE_INSERT);
								CachePvpRaidLose *pvp_data = cache_pvp_raid_lose__unpack(NULL, msg_len, proto->data);
								if (!pvp_data)
								{
									LOG_ERR("[%s:%d] player [%lu] unpack req failed", __FUNCTION__, __LINE__, proto->player_id);
									player_offline_cache__free_unpacked(cache_info, NULL);
									return;
								}
								cache_info->n_pvp_lose++;
								cache_info->pvp_lose = (CachePvpRaidLose **)realloc(cache_info->pvp_lose, cache_info->n_pvp_lose * sizeof(CachePvpRaidLose));

								cache_info->pvp_lose[cache_info->n_pvp_lose - 1] = pvp_data;

								data_len = player_offline_cache__pack(cache_info, (uint8_t*)cache_data);
								save_player_cache(proto->player_id, cache_data, data_len);

								cache_info->n_pvp_lose--;
								cache_pvp_raid_lose__free_unpacked(pvp_data, NULL);
	}
		break;
	case CACHE_PVP_RAID_WIN:
	{
							   int msg_len = get_len() - sizeof(PROTO_PLAYER_CACHE_INSERT);
							   CachePvpRaidWin *pvp_data = cache_pvp_raid_win__unpack(NULL, msg_len, proto->data);
							   if (!pvp_data)
							   {
								   LOG_ERR("[%s:%d] player [%lu] unpack req failed", __FUNCTION__, __LINE__, proto->player_id);
								   player_offline_cache__free_unpacked(cache_info, NULL);
								   return;
							   }
							   cache_info->n_pvp_win++;
							   cache_info->pvp_win = (CachePvpRaidWin **)realloc(cache_info->pvp_win, cache_info->n_pvp_win * sizeof(CachePvpRaidWin));

							   cache_info->pvp_win[cache_info->n_pvp_win - 1] = pvp_data;

							   data_len = player_offline_cache__pack(cache_info, (uint8_t*)cache_data);
							   save_player_cache(proto->player_id, cache_data, data_len);

							   cache_info->n_pvp_win--;
							   cache_pvp_raid_win__free_unpacked(pvp_data, NULL);
	}
		break;
	}

	//释放分配的内存
	player_offline_cache__free_unpacked(cache_info, NULL);
}


void conn_node_friendsrv::handle_friend_info_request()
{
	PROTO_HEAD *head = get_head();
	EXTERN_DATA *extern_data = get_extern_data(head);

	int ret = 0;
	FriendPlayer *player = NULL;
	std::map<uint64_t, PlayerRedisInfo*> redis_players;
	AutoReleaseBatchFriendPlayer arb_friend;
	AutoReleaseBatchRedisPlayer t1;
	do
	{
		player = get_friend_player(extern_data->player_id);
		if (!player)
		{
			ret = ERROR_ID_SERVER;
			LOG_ERR("[%s:%d] player[%lu] get friend failed", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}
		arb_friend.push_back(player);

		if (get_all_friend_redis_player(player, redis_players, t1) != 0)
		{
			ret = ERROR_ID_SERVER;
			LOG_ERR("[%s:%d] player[%lu] get_all_friend_redis_player", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}
	} while (0);

	FriendInfoAnswer resp;
	friend_info_answer__init(&resp);

	FriendGroupData  fix_data[FRIEND_LIST_TYPE__L_APPLY];
	FriendGroupData* fix_point[FRIEND_LIST_TYPE__L_APPLY];
	FriendGroupData  custom_data[MAX_FRIEND_GROUP_NUM];
	FriendGroupData* custom_point[MAX_FRIEND_GROUP_NUM];
	FriendPlayerBriefData  recent_data[MAX_FRIEND_RECENT_NUM];
	FriendPlayerBriefData* recent_point[MAX_FRIEND_RECENT_NUM];
	AttrData  recent_attr_data[MAX_FRIEND_RECENT_NUM][MAX_FRIEND_UNIT_ATTR_NUM];
	AttrData* recent_attr_point[MAX_FRIEND_RECENT_NUM][MAX_FRIEND_UNIT_ATTR_NUM];
	FriendPlayerBriefData  contact_data[MAX_FRIEND_CONTACT_NUM];
	FriendPlayerBriefData* contact_point[MAX_FRIEND_CONTACT_NUM];
	AttrData  contact_attr_data[MAX_FRIEND_CONTACT_NUM][MAX_FRIEND_UNIT_ATTR_NUM];
	AttrData* contact_attr_point[MAX_FRIEND_CONTACT_NUM][MAX_FRIEND_UNIT_ATTR_NUM];
	FriendPlayerBriefData  block_data[MAX_FRIEND_BLOCK_NUM];
	FriendPlayerBriefData* block_point[MAX_FRIEND_BLOCK_NUM];
	AttrData  block_attr_data[MAX_FRIEND_BLOCK_NUM][MAX_FRIEND_UNIT_ATTR_NUM];
	AttrData* block_attr_point[MAX_FRIEND_BLOCK_NUM][MAX_FRIEND_UNIT_ATTR_NUM];
	FriendPlayerBriefData  enemy_data[MAX_FRIEND_ENEMY_NUM];
	FriendPlayerBriefData* enemy_point[MAX_FRIEND_ENEMY_NUM];
	AttrData  enemy_attr_data[MAX_FRIEND_ENEMY_NUM][MAX_FRIEND_UNIT_ATTR_NUM];
	AttrData* enemy_attr_point[MAX_FRIEND_ENEMY_NUM][MAX_FRIEND_UNIT_ATTR_NUM];
	FriendPlayerBriefData  apply_data[MAX_FRIEND_APPLY_NUM];
	FriendPlayerBriefData* apply_point[MAX_FRIEND_APPLY_NUM];
	AttrData  apply_attr_data[MAX_FRIEND_APPLY_NUM][MAX_FRIEND_UNIT_ATTR_NUM];
	AttrData* apply_attr_point[MAX_FRIEND_APPLY_NUM][MAX_FRIEND_UNIT_ATTR_NUM];

	std::map<uint32_t, std::vector<FriendUnit*> > group_friends;
	resp.result = ret;
	if (player)
	{
		resp.n_fixs = 0;
		resp.fixs = fix_point;
		for (int i = 0; i < FRIEND_LIST_TYPE__L_APPLY; ++i)
		{
			fix_point[i] = &fix_data[i];
			friend_group_data__init(&fix_data[i]);
			resp.n_fixs++;
		}

		FriendGroupData *recent_group = &fix_data[0];
		recent_group->groupid = FRIEND_LIST_TYPE__L_RECENT;
		recent_group->n_players = 0;
		recent_group->players = recent_point;
		for (int i = MAX_FRIEND_RECENT_NUM - 1; i >= 0; --i)
		{
			uint64_t player_id = player->recents[i];
			if (player_id == 0)
			{
				continue;
			}

			PlayerRedisInfo *redis_player = find_redis_from_map(redis_players, player_id);
			if (!redis_player)
			{
				continue;
			}

			recent_point[recent_group->n_players] = &recent_data[recent_group->n_players];
			friend_player_brief_data__init(&recent_data[recent_group->n_players]);
			recent_data[recent_group->n_players].attrs = recent_attr_point[recent_group->n_players];
			recent_data[recent_group->n_players].n_attrs = 0;
			for (int j = 0; j < MAX_FRIEND_UNIT_ATTR_NUM; ++j)
			{
				recent_attr_point[recent_group->n_players][j] = &recent_attr_data[recent_group->n_players][j];
				attr_data__init(&recent_attr_data[recent_group->n_players][j]);
			}

			set_proto_friend(*redis_player, recent_data[recent_group->n_players]);
			recent_data[recent_group->n_players].playerid = player_id;
			recent_data[recent_group->n_players].closeness = get_friend_closeness(player, player_id);
			recent_group->n_players++;
		}

		FriendGroupData *contact_group = &fix_data[1];
		contact_group->groupid = FRIEND_LIST_TYPE__L_CONTACT;
		contact_group->n_players = 0;
		contact_group->players = contact_point;
		for (int i = 0; i < MAX_FRIEND_CONTACT_NUM; ++i)
		{
			uint64_t player_id = player->contacts[i].player_id;
			if (player_id == 0)
			{
				break;
			}
			if (player->contacts[i].group_id != 0)
			{
				group_friends[player->contacts[i].group_id].push_back(&player->contacts[i]);
				continue;
			}

			PlayerRedisInfo *redis_player = find_redis_from_map(redis_players, player_id);
			if (!redis_player)
			{
				continue;
			}

			contact_point[contact_group->n_players] = &contact_data[contact_group->n_players];
			friend_player_brief_data__init(&contact_data[contact_group->n_players]);
			contact_data[contact_group->n_players].attrs = contact_attr_point[contact_group->n_players];
			contact_data[contact_group->n_players].n_attrs = 0;
			for (int j = 0; j < MAX_FRIEND_UNIT_ATTR_NUM; ++j)
			{
				contact_attr_point[contact_group->n_players][j] = &contact_attr_data[contact_group->n_players][j];
				attr_data__init(&contact_attr_data[contact_group->n_players][j]);
			}

			set_proto_friend(*redis_player, contact_data[contact_group->n_players]);
			contact_data[contact_group->n_players].playerid = player_id;
			contact_data[contact_group->n_players].closeness = player->contacts[i].closeness;
			contact_group->n_players++;
		}

		FriendGroupData *block_group = &fix_data[2];
		block_group->groupid = FRIEND_LIST_TYPE__L_BLOCK;
		block_group->n_players = 0;
		block_group->players = block_point;
		for (int i = 0; i < MAX_FRIEND_BLOCK_NUM; ++i)
		{
			uint64_t player_id = player->blocks[i].player_id;
			if (player_id == 0)
			{
				break;
			}

			PlayerRedisInfo *redis_player = find_redis_from_map(redis_players, player_id);
			if (!redis_player)
			{
				continue;
			}

			block_point[block_group->n_players] = &block_data[block_group->n_players];
			friend_player_brief_data__init(&block_data[block_group->n_players]);
			block_data[block_group->n_players].attrs = block_attr_point[block_group->n_players];
			block_data[block_group->n_players].n_attrs = 0;
			for (int j = 0; j < MAX_FRIEND_UNIT_ATTR_NUM; ++j)
			{
				block_attr_point[block_group->n_players][j] = &block_attr_data[block_group->n_players][j];
				attr_data__init(&block_attr_data[block_group->n_players][j]);
			}

			set_proto_friend(*redis_player, block_data[block_group->n_players]);
			block_data[block_group->n_players].playerid = player_id;
			block_data[block_group->n_players].closeness = player->blocks[i].closeness;
			block_group->n_players++;
		}

		FriendGroupData *enemy_group = &fix_data[3];
		enemy_group->groupid = FRIEND_LIST_TYPE__L_ENEMY;
		enemy_group->n_players = 0;
		enemy_group->players = enemy_point;
		for (int i = 0; i < MAX_FRIEND_ENEMY_NUM; ++i)
		{
			uint64_t player_id = player->enemies[i].player_id;
			if (player_id == 0)
			{
				break;
			}

			PlayerRedisInfo *redis_player = find_redis_from_map(redis_players, player_id);
			if (!redis_player)
			{
				continue;
			}

			enemy_point[enemy_group->n_players] = &enemy_data[enemy_group->n_players];
			friend_player_brief_data__init(&enemy_data[enemy_group->n_players]);
			enemy_data[enemy_group->n_players].attrs = enemy_attr_point[enemy_group->n_players];
			enemy_data[enemy_group->n_players].n_attrs = 0;
			for (int j = 0; j < MAX_FRIEND_UNIT_ATTR_NUM; ++j)
			{
				enemy_attr_point[enemy_group->n_players][j] = &enemy_attr_data[enemy_group->n_players][j];
				attr_data__init(&enemy_attr_data[enemy_group->n_players][j]);
			}

			set_proto_friend(*redis_player, enemy_data[enemy_group->n_players]);
			enemy_data[enemy_group->n_players].playerid = player_id;
			enemy_data[enemy_group->n_players].closeness = get_friend_closeness(player, player_id);
			enemy_data[enemy_group->n_players].has_tracktime = true;
			enemy_data[enemy_group->n_players].tracktime = player->enemies[i].track_time;
			enemy_group->n_players++;
		}

		FriendGroupData *apply_group = &fix_data[4];
		apply_group->groupid = FRIEND_LIST_TYPE__L_APPLY;
		apply_group->n_players = 0;
		apply_group->players = apply_point;
		for (int i = 0; i < MAX_FRIEND_APPLY_NUM; ++i)
		{
			uint64_t player_id = player->applys[i];
			if (player_id == 0)
			{
				break;
			}

			PlayerRedisInfo *redis_player = find_redis_from_map(redis_players, player_id);
			if (!redis_player)
			{
				continue;
			}

			apply_point[apply_group->n_players] = &apply_data[apply_group->n_players];
			friend_player_brief_data__init(&apply_data[apply_group->n_players]);
			apply_data[apply_group->n_players].attrs = apply_attr_point[apply_group->n_players];
			apply_data[apply_group->n_players].n_attrs = 0;
			for (int j = 0; j < MAX_FRIEND_UNIT_ATTR_NUM; ++j)
			{
				apply_attr_point[apply_group->n_players][j] = &apply_attr_data[apply_group->n_players][j];
				attr_data__init(&apply_attr_data[apply_group->n_players][j]);
			}

			set_proto_friend(*redis_player, apply_data[apply_group->n_players]);
			apply_data[apply_group->n_players].playerid = player_id;
			apply_group->n_players++;
		}

		size_t contact_cnt = contact_group->n_players;
		resp.n_customs = 0;
		resp.customs = custom_point;
		for (int i = 0; i < MAX_FRIEND_GROUP_NUM; ++i)
		{
			uint32_t group_id = player->groups[i].group_id;
			if (group_id == 0)
			{
				break;
			}

			custom_point[resp.n_customs] = &custom_data[resp.n_customs];
			friend_group_data__init(&custom_data[resp.n_customs]);
			FriendGroupData *custom_group = &custom_data[resp.n_customs];
			resp.n_customs++;

			custom_group->groupid = group_id;
			custom_group->createtime = player->groups[i].create_time;
			custom_group->name = player->groups[i].group_name;
			custom_group->n_players = 0;
			custom_group->players = &contact_point[contact_cnt];
			std::map<uint32_t, std::vector<FriendUnit *> >::iterator iter_map = group_friends.find(group_id);
			if (iter_map == group_friends.end())
			{
				continue;
			}

			for (std::vector<FriendUnit*>::iterator iter = iter_map->second.begin(); iter != iter_map->second.end(); ++iter)
			{
				FriendUnit *unit = *iter;
				PlayerRedisInfo *redis_player = find_redis_from_map(redis_players, unit->player_id);
				if (!redis_player)
				{
					continue;
				}
				contact_point[contact_cnt] = &contact_data[contact_cnt];
				friend_player_brief_data__init(&contact_data[contact_cnt]);
				contact_data[contact_cnt].attrs = contact_attr_point[contact_cnt];
				contact_data[contact_cnt].n_attrs = 0;
				for (int j = 0; j < MAX_FRIEND_UNIT_ATTR_NUM; ++j)
				{
					contact_attr_point[contact_cnt][j] = &contact_attr_data[contact_cnt][j];
					attr_data__init(&contact_attr_data[contact_cnt][j]);
				}

				set_proto_friend(*redis_player, contact_data[contact_cnt]);
				contact_data[contact_cnt].playerid = unit->player_id;
				contact_data[contact_cnt].closeness = unit->closeness;
				custom_group->n_players++;
				contact_cnt++;
			}
		}

		resp.contact_extend = player->contact_extend;
		resp.autoacceptapply = player->auto_accept_apply;
	}

	fast_send_msg(&connecter, extern_data, MSG_ID_FRIEND_INFO_ANSWER, friend_info_answer__pack, resp);

	// for (std::map<uint64_t, PlayerRedisInfo*>::iterator iter = redis_players.begin(); iter != redis_players.end(); ++iter)
	// {
	// 	player_redis_info__free_unpacked(iter->second, NULL);
	// }
}

void conn_node_friendsrv::handle_friend_add_contact_request()
{
	PROTO_HEAD *head = get_head();
	EXTERN_DATA *extern_data = get_extern_data(head);

	FriendOperateRequest *req = friend_operate_request__unpack(NULL, get_data_len(), get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] player[%lu] unpack failed", __FUNCTION__, __LINE__, extern_data->player_id);
		return;
	}

	uint64_t target_id = req->playerid;
	friend_operate_request__free_unpacked(req, NULL);

	int ret = 0;
	AutoReleaseBatchFriendPlayer arb_friend;
	do
	{
		FriendPlayer *player = get_friend_player(extern_data->player_id);
		if (!player)
		{
			ret = ERROR_ID_SERVER;
			LOG_ERR("[%s:%d] player[%lu] get friend failed", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}
		arb_friend.push_back(player);

		FriendListChangeInfo change_info;
		ret = add_contact(player, target_id, change_info, true, false);
		if (ret == 0)
		{
			notify_friend_list_change(player, change_info);
			save_friend_player(player);
		}
	} while (0);

	FriendOperateAnswer resp;
	friend_operate_answer__init(&resp);

	resp.result = ret;
	resp.playerid = target_id;

	fast_send_msg(&connecter, extern_data, MSG_ID_FRIEND_ADD_CONTACT_ANSWER, friend_operate_answer__pack, resp);
}

void conn_node_friendsrv::handle_friend_del_contact_request()
{
	PROTO_HEAD *head = get_head();
	EXTERN_DATA *extern_data = get_extern_data(head);

	FriendOperateRequest *req = friend_operate_request__unpack(NULL, get_data_len(), get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] player[%lu] unpack failed", __FUNCTION__, __LINE__, extern_data->player_id);
		return;
	}

	uint64_t target_id = req->playerid;
	friend_operate_request__free_unpacked(req, NULL);

	int ret = 0;
	AutoReleaseBatchFriendPlayer arb_friend;
	do
	{
		FriendPlayer *player = get_friend_player(extern_data->player_id);
		if (!player)
		{
			ret = ERROR_ID_SERVER;
			LOG_ERR("[%s:%d] player[%lu] get friend failed", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}
		arb_friend.push_back(player);

		FriendListChangeInfo change_info;
		ret = del_contact(player, target_id, change_info);
		if (ret != 0)
		{
			break;
		}

		notify_friend_list_change(player, change_info);
		save_friend_player(player);
	} while (0);

	FriendOperateAnswer resp;
	friend_operate_answer__init(&resp);

	resp.result = ret;
	resp.playerid = target_id;

	fast_send_msg(&connecter, extern_data, MSG_ID_FRIEND_DEL_CONTACT_ANSWER, friend_operate_answer__pack, resp);
}

void conn_node_friendsrv::handle_friend_add_block_request()
{
	PROTO_HEAD *head = get_head();
	EXTERN_DATA *extern_data = get_extern_data(head);

	FriendOperateRequest *req = friend_operate_request__unpack(NULL, get_data_len(), get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] player[%lu] unpack failed", __FUNCTION__, __LINE__, extern_data->player_id);
		return;
	}

	uint64_t target_id = req->playerid;
	friend_operate_request__free_unpacked(req, NULL);

	int ret = 0;
	AutoReleaseBatchFriendPlayer arb_friend;
	do
	{
		FriendPlayer *player = get_friend_player(extern_data->player_id);
		if (!player)
		{
			ret = ERROR_ID_SERVER;
			LOG_ERR("[%s:%d] player[%lu] get friend failed", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}
		arb_friend.push_back(player);

		ret = add_block(player, target_id);
	} while (0);

	FriendOperateAnswer resp;
	friend_operate_answer__init(&resp);

	resp.result = ret;
	resp.playerid = target_id;

	fast_send_msg(&connecter, extern_data, MSG_ID_FRIEND_ADD_BLOCK_ANSWER, friend_operate_answer__pack, resp);
}

void conn_node_friendsrv::handle_friend_del_block_request()
{
	PROTO_HEAD *head = get_head();
	EXTERN_DATA *extern_data = get_extern_data(head);

	FriendOperateRequest *req = friend_operate_request__unpack(NULL, get_data_len(), get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] player[%lu] unpack failed", __FUNCTION__, __LINE__, extern_data->player_id);
		return;
	}

	uint64_t target_id = req->playerid;
	friend_operate_request__free_unpacked(req, NULL);

	int ret = 0;
	AutoReleaseBatchFriendPlayer arb_friend;
	do
	{
		FriendPlayer *player = get_friend_player(extern_data->player_id);
		if (!player)
		{
			ret = ERROR_ID_SERVER;
			LOG_ERR("[%s:%d] player[%lu] get friend failed", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}
		arb_friend.push_back(player);

		FriendListChangeInfo change_info;
		ret = del_block(player, target_id, change_info);
		if (ret != 0)
		{
			break;
		}

		notify_friend_list_change(player, change_info);
		save_friend_player(player);
	} while (0);

	FriendOperateAnswer resp;
	friend_operate_answer__init(&resp);

	resp.result = ret;
	resp.playerid = target_id;

	fast_send_msg(&connecter, extern_data, MSG_ID_FRIEND_DEL_BLOCK_ANSWER, friend_operate_answer__pack, resp);
}

void conn_node_friendsrv::handle_friend_del_enemy_request()
{
	PROTO_HEAD *head = get_head();
	EXTERN_DATA *extern_data = get_extern_data(head);

	FriendOperateRequest *req = friend_operate_request__unpack(NULL, get_data_len(), get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] player[%lu] unpack failed", __FUNCTION__, __LINE__, extern_data->player_id);
		return;
	}

	uint64_t target_id = req->playerid;
	friend_operate_request__free_unpacked(req, NULL);

	int ret = 0;
	AutoReleaseBatchFriendPlayer arb_friend;
	do
	{
		FriendPlayer *player = get_friend_player(extern_data->player_id);
		if (!player)
		{
			ret = ERROR_ID_SERVER;
			LOG_ERR("[%s:%d] player[%lu] get friend failed", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}
		arb_friend.push_back(player);

		FriendListChangeInfo change_info;
		ret = del_enemy(player, target_id, change_info);
		if (ret != 0)
		{
			break;
		}

		notify_friend_list_change(player, change_info);
		save_friend_player(player);
	} while (0);

	FriendOperateAnswer resp;
	friend_operate_answer__init(&resp);

	resp.result = ret;
	resp.playerid = target_id;

	fast_send_msg(&connecter, extern_data, MSG_ID_FRIEND_DEL_ENEMY_ANSWER, friend_operate_answer__pack, resp);
}

void conn_node_friendsrv::handle_friend_create_group_request()
{
	PROTO_HEAD *head = get_head();
	EXTERN_DATA *extern_data = get_extern_data(head);

	FriendCreateGroupRequest *req = friend_create_group_request__unpack(NULL, get_data_len(), get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] player[%lu] unpack failed", __FUNCTION__, __LINE__, extern_data->player_id);
		return;
	}

	std::vector<uint64_t> members;
	for (size_t i = 0; i < req->n_playerids; ++i)
	{
		members.push_back(req->playerids[i]);
	}
	std::string group_name(req->name);
	friend_create_group_request__free_unpacked(req, NULL);

	int ret = 0;
	AutoReleaseBatchFriendPlayer arb_friend;
	FriendPlayer *player = NULL;
	FriendGroup *pGroup = NULL;
	std::vector<int> member_idxs;
	AutoReleaseBatchRedisPlayer t1;
	std::map<uint64_t, PlayerRedisInfo*> redis_players;
	do
	{
		player = get_friend_player(extern_data->player_id);
		if (!player)
		{
			ret = ERROR_ID_SERVER;
			LOG_ERR("[%s:%d] player[%lu] get friend failed", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}
		arb_friend.push_back(player);

		FriendListChangeInfo change_info;
		//检查成员是否在好友分组
		for (std::vector<uint64_t>::iterator iter = members.begin(); iter != members.end(); ++iter)
		{
			int idx = get_contact_idx(player, *iter);
			if (idx < 0)
			{
				ret = ERROR_ID_FRIEND_NOT_IN_LIST;
				LOG_ERR("[%s:%d] player[%lu] friend %lu", __FUNCTION__, __LINE__, extern_data->player_id, *iter);
				break;
			}

			if (player->contacts[idx].group_id != 0)
			{
				ret = ERROR_ID_FRIEND_NOT_IN_LIST;
				LOG_ERR("[%s:%d] player[%lu] friend %lu in group %u", __FUNCTION__, __LINE__, extern_data->player_id, *iter, player->contacts[idx].group_id);
				break;
			}

			member_idxs.push_back(idx);
		}

		if (ret != 0)
		{
			break;
		}

		//检查名字长度
		if (group_name.size() == 0 || group_name.size() >= MAX_FRIEND_GROUP_NAME_LEN)
		{
			ret = ERROR_ID_FRIEND_GROUP_NAME_LEN;
			LOG_ERR("[%s:%d] player[%lu] group name length:%lu", __FUNCTION__, __LINE__, extern_data->player_id, group_name.size());
			break;
		}

		//检查分组数量，以及是否重名
		pGroup = NULL;
		int group_limit_num = get_group_limit_num();
		for (int i = 0; i < group_limit_num; ++i)
		{
			if (player->groups[i].group_id == 0)
			{
				pGroup = &player->groups[i];
				break;
			}

			if (group_name.compare(player->groups[i].group_name) == 0)
			{
				pGroup = &player->groups[i];
				break;
			}
		}

		if (!pGroup)
		{
			ret = 190500217;
			LOG_ERR("[%s:%d] player[%lu] group num max", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}

		if (pGroup->group_id > 0)
		{
			ret = 190500215;
			LOG_ERR("[%s:%d] player[%lu] group name used, group_name:%s", __FUNCTION__, __LINE__, extern_data->player_id, pGroup->group_name);
			break;
		}

		pGroup->group_id = get_new_group_id(player);
		pGroup->create_time = time_helper::get_cached_time() / 1000;
		strcpy(pGroup->group_name, group_name.c_str());

		std::set<uint64_t> playerIds;
		//把成员从好友分组移到新分组
		for (std::vector<int>::iterator iter = member_idxs.begin(); iter != member_idxs.end(); ++iter)
		{
			FriendListDel contact_del;
			contact_del.group_id = FRIEND_LIST_TYPE__L_CONTACT;
			contact_del.player_id = player->contacts[*iter].player_id;
			change_info.dels.push_back(contact_del);

			player->contacts[*iter].group_id = pGroup->group_id;
			playerIds.insert(player->contacts[*iter].player_id);
		}

		if (members.size() > 0)
		{
			get_more_redis_player(playerIds, redis_players, conn_node_friendsrv::server_key, sg_redis_client, t1);
			notify_friend_list_change(player, change_info);
		}
		save_friend_player(player);
	} while (0);

	FriendCreateGroupAnswer resp;
	friend_create_group_answer__init(&resp);

	FriendGroupData group_data;
	friend_group_data__init(&group_data);

	FriendPlayerBriefData member_data[MAX_FRIEND_CONTACT_NUM];
	FriendPlayerBriefData* member_point[MAX_FRIEND_CONTACT_NUM];
	AttrData  member_attr_data[MAX_FRIEND_CONTACT_NUM][MAX_FRIEND_UNIT_ATTR_NUM];
	AttrData* member_attr_point[MAX_FRIEND_CONTACT_NUM][MAX_FRIEND_UNIT_ATTR_NUM];

	resp.result = ret;
	if (ret == 0 && player != NULL && pGroup != NULL)
	{
		resp.info = &group_data;
		group_data.groupid = pGroup->group_id;
		group_data.createtime = pGroup->create_time;
		group_data.name = pGroup->group_name;
		group_data.n_players = 0;
		group_data.players = member_point;
		for (std::vector<int>::iterator iter = member_idxs.begin(); iter != member_idxs.end(); ++iter)
		{
			FriendUnit *unit = &player->contacts[*iter];
			member_point[group_data.n_players] = &member_data[group_data.n_players];
			friend_player_brief_data__init(&member_data[group_data.n_players]);
			member_data[group_data.n_players].attrs = member_attr_point[group_data.n_players];
			member_data[group_data.n_players].n_attrs = 0;
			for (int j = 0; j < MAX_FRIEND_UNIT_ATTR_NUM; ++j)
			{
				member_attr_point[group_data.n_players][j] = &member_attr_data[group_data.n_players][j];
				attr_data__init(&member_attr_data[group_data.n_players][j]);
			}

			PlayerRedisInfo *redis_player = find_redis_from_map(redis_players, unit->player_id);
			if (redis_player)
			{
				set_proto_friend(*redis_player, member_data[group_data.n_players]);
			}
			member_data[group_data.n_players].playerid = unit->player_id;
			member_data[group_data.n_players].closeness = unit->closeness;
			group_data.n_players++;
		}
	}

	fast_send_msg(&connecter, extern_data, MSG_ID_FRIEND_CREATE_GROUP_ANSWER, friend_create_group_answer__pack, resp);

	// for (std::map<uint64_t, PlayerRedisInfo*>::iterator iter = redis_players.begin(); iter != redis_players.end(); ++iter)
	// {
	// 	player_redis_info__free_unpacked(iter->second, NULL);
	// }
}

void conn_node_friendsrv::handle_friend_edit_group_request()
{
	PROTO_HEAD *head = get_head();
	EXTERN_DATA *extern_data = get_extern_data(head);

	FriendEditGroupRequest *req = friend_edit_group_request__unpack(NULL, get_data_len(), get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] player[%lu] unpack failed", __FUNCTION__, __LINE__, extern_data->player_id);
		return;
	}

	std::vector<uint64_t> members;
	for (size_t i = 0; i < req->n_playerids; ++i)
	{
		members.push_back(req->playerids[i]);
	}
	std::string group_name(req->name);
	uint32_t group_id = req->groupid;
	friend_edit_group_request__free_unpacked(req, NULL);

	int ret = 0;
	AutoReleaseBatchFriendPlayer arb_friend;
	FriendPlayer *player = NULL;
	FriendGroup *pGroup = NULL;
	std::vector<int> member_adds; //分组新增成员索引
	do
	{
		player = get_friend_player(extern_data->player_id);
		if (!player)
		{
			ret = ERROR_ID_SERVER;
			LOG_ERR("[%s:%d] player[%lu] get friend failed", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}
		arb_friend.push_back(player);

		if (group_id == 0)
		{
			ret = ERROR_ID_FRIEND_GROUP_ID;
			break;
		}

		FriendListChangeInfo change_info;
		//检查成员是否在好友分组
		for (std::vector<uint64_t>::iterator iter = members.begin(); iter != members.end(); ++iter)
		{
			int idx = get_contact_idx(player, *iter);
			if (idx < 0)
			{
				ret = ERROR_ID_FRIEND_NOT_IN_LIST;
				LOG_ERR("[%s:%d] player[%lu] friend %lu", __FUNCTION__, __LINE__, extern_data->player_id, *iter);
				break;
			}

			if (player->contacts[idx].group_id != 0)
			{
				ret = ERROR_ID_FRIEND_NOT_IN_LIST;
				LOG_ERR("[%s:%d] player[%lu] can't move friend %lu in group %u, edit_group:%u", __FUNCTION__, __LINE__, extern_data->player_id, *iter, player->contacts[idx].group_id, group_id);
				break;
			}

			member_adds.push_back(idx);
		}

		if (ret != 0)
		{
			break;
		}

		//检查名字长度
		if (group_name.size() == 0 || group_name.size() >= MAX_FRIEND_GROUP_NAME_LEN)
		{
			ret = ERROR_ID_FRIEND_GROUP_NAME_LEN;
			LOG_ERR("[%s:%d] player[%lu] group name length:%lu", __FUNCTION__, __LINE__, extern_data->player_id, group_name.size());
			break;
		}

		//检查分组是否存在，以及是否重名
		pGroup = NULL;
		bool bNameExist = false;
		int group_limit_num = get_group_limit_num();
		for (int i = 0; i < group_limit_num; ++i)
		{
			if (player->groups[i].group_id == 0)
			{
				continue;
			}

			if (player->groups[i].group_id == group_id)
			{
				pGroup = &player->groups[i];
			}
			else if (group_name.compare(player->groups[i].group_name) == 0)
			{
				bNameExist = true;
			}
		}

		if (!pGroup)
		{
			ret = ERROR_ID_FRIEND_GROUP_ID;
			LOG_ERR("[%s:%d] player[%lu] group not found, group_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, group_id);
			break;
		}

		if (bNameExist)
		{
			ret = 190500215;
			LOG_ERR("[%s:%d] player[%lu] group name used, group_name:%s", __FUNCTION__, __LINE__, extern_data->player_id, group_name.c_str());
			break;
		}

		strcpy(pGroup->group_name, group_name.c_str());

		//移动要新增的成员
		for (std::vector<int>::iterator iter = member_adds.begin(); iter != member_adds.end(); ++iter)
		{
			FriendUnit *unit = &player->contacts[*iter];
			FriendListDel contact_del;
			contact_del.group_id = FRIEND_LIST_TYPE__L_CONTACT;
			contact_del.player_id = unit->player_id;
			change_info.dels.push_back(contact_del);

			FriendListAdd group_add;
			group_add.group_id = group_id;
			group_add.player_id = unit->player_id;
			group_add.closeness = unit->closeness;
			change_info.adds.push_back(group_add);

			unit->group_id = pGroup->group_id;
		}

		notify_friend_list_change(player, change_info);
		save_friend_player(player);
	} while (0);

	FriendEditGroupAnswer resp;
	friend_edit_group_answer__init(&resp);

	resp.result = ret;
	resp.groupid = group_id;
	resp.name = const_cast<char*>(group_name.c_str());

	fast_send_msg(&connecter, extern_data, MSG_ID_FRIEND_EDIT_GROUP_ANSWER, friend_edit_group_answer__pack, resp);
}

void conn_node_friendsrv::handle_friend_remove_group_request()
{
	PROTO_HEAD *head = get_head();
	EXTERN_DATA *extern_data = get_extern_data(head);

	FriendRemoveGroupRequest *req = friend_remove_group_request__unpack(NULL, get_data_len(), get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] player[%lu] unpack failed", __FUNCTION__, __LINE__, extern_data->player_id);
		return;
	}

	uint32_t group_id = req->groupid;
	friend_remove_group_request__free_unpacked(req, NULL);

	int ret = 0;
	AutoReleaseBatchFriendPlayer arb_friend;
	FriendPlayer *player = NULL;
	std::vector<int> member_idxs; //分组所有成员索引
	do
	{
		player = get_friend_player(extern_data->player_id);
		if (!player)
		{
			ret = ERROR_ID_SERVER;
			LOG_ERR("[%s:%d] player[%lu] get friend failed", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}
		arb_friend.push_back(player);

		if (group_id == 0)
		{
			ret = ERROR_ID_FRIEND_GROUP_ID;
			break;
		}

		FriendListChangeInfo change_info;

		//检查分组是否存在
		int group_idx = -1;
		int group_limit_num = get_group_limit_num();
		for (int i = 0; i < group_limit_num; ++i)
		{
			if (player->groups[i].group_id == group_id)
			{
				group_idx = i;
				break;
			}
		}

		if (group_idx < 0)
		{
			ret = ERROR_ID_FRIEND_GROUP_ID;
			LOG_ERR("[%s:%d] player[%lu] group not found, group_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, group_id);
			break;
		}

		//找出分组所有成员
		int contact_limit_num = get_contact_limit_num(player);
		for (int i = 0; i < contact_limit_num; ++i)
		{
			FriendUnit *unit = &player->contacts[i];
			if (unit->player_id == 0)
			{
				break;
			}
			if (unit->group_id == group_id)
			{
				member_idxs.push_back(i);
			}
		}

		//移除成员
		for (std::vector<int>::iterator iter = member_idxs.begin(); iter != member_idxs.end(); ++iter)
		{
			FriendUnit *unit = &player->contacts[*iter];
			FriendListAdd contact_add;
			contact_add.group_id = FRIEND_LIST_TYPE__L_CONTACT;
			contact_add.player_id = unit->player_id;
			contact_add.closeness = unit->closeness;
			change_info.adds.push_back(contact_add);

			unit->group_id = 0;
		}

		//删除分组
		{
			int last_idx = group_limit_num - 1;
			if (group_idx < last_idx)
			{
				memmove(&player->groups[group_idx], &player->groups[group_idx + 1], (last_idx - group_idx) * sizeof(FriendGroup));
			}
			memset(&player->groups[last_idx], 0, sizeof(FriendGroup));
		}

		notify_friend_list_change(player, change_info);
		save_friend_player(player);
	} while (0);

	CommAnswer resp;
	comm_answer__init(&resp);

	resp.result = ret;
	fast_send_msg(&connecter, extern_data, MSG_ID_FRIEND_REMOVE_GROUP_ANSWER, comm_answer__pack, resp);
}

void conn_node_friendsrv::handle_friend_move_player_group_request()
{
	PROTO_HEAD *head = get_head();
	EXTERN_DATA *extern_data = get_extern_data(head);

	FriendMovePlayerGroupRequest *req = friend_move_player_group_request__unpack(NULL, get_data_len(), get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] player[%lu] unpack failed", __FUNCTION__, __LINE__, extern_data->player_id);
		return;
	}

	uint64_t target_id = req->playerid;
	uint32_t group_id = req->togroup;
	friend_move_player_group_request__free_unpacked(req, NULL);

	int ret = 0;
	AutoReleaseBatchFriendPlayer arb_friend;
	FriendPlayer *player = NULL;
	do
	{
		player = get_friend_player(extern_data->player_id);
		if (!player)
		{
			ret = ERROR_ID_SERVER;
			LOG_ERR("[%s:%d] player[%lu] get friend failed", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}
		arb_friend.push_back(player);

		FriendListChangeInfo change_info;

		//检查分组是否存在
		if (group_id != 0)
		{
			int group_idx = -1;
			int group_limit_num = get_group_limit_num();
			for (int i = 0; i < group_limit_num; ++i)
			{
				if (player->groups[i].group_id == group_id)
				{
					group_idx = i;
					break;
				}
			}

			if (group_idx < 0)
			{
				ret = ERROR_ID_FRIEND_GROUP_ID;
				LOG_ERR("[%s:%d] player[%lu] group not found, group_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, group_id);
				break;
			}
		}

		//找出成员
		int member_idx = get_contact_idx(player, target_id);
		if (member_idx < 0)
		{
			ret = ERROR_ID_FRIEND_ID;
			LOG_ERR("[%s:%d] player[%lu] friend, target_id:%lu, group_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, target_id, group_id);
			break;
		}

		FriendUnit *unit = &player->contacts[member_idx];
		if (unit->group_id == group_id)
		{
			ret = ERROR_ID_FRIEND_GROUP_ID;
			LOG_ERR("[%s:%d] player[%lu] target in group, target_id:%lu, group_id:%u", __FUNCTION__, __LINE__, extern_data->player_id, target_id, group_id);
			break;
		}

		//移动成员
		{
			FriendListAdd group_add;
			group_add.group_id = (group_id == 0 ? FRIEND_LIST_TYPE__L_CONTACT : group_id);
			group_add.player_id = unit->player_id;
			group_add.closeness = unit->closeness;
			change_info.adds.push_back(group_add);

			FriendListDel group_del;
			group_del.group_id = (unit->group_id == 0 ? FRIEND_LIST_TYPE__L_CONTACT : unit->group_id);
			group_del.player_id = unit->player_id;
			change_info.dels.push_back(group_del);

			unit->group_id = group_id;
		}

		notify_friend_list_change(player, change_info);
		save_friend_player(player);
	} while (0);

	CommAnswer resp;
	comm_answer__init(&resp);

	resp.result = ret;
	fast_send_msg(&connecter, extern_data, MSG_ID_FRIEND_MOVE_PLAYER_GROUP_ANSWER, comm_answer__pack, resp);
}

void conn_node_friendsrv::handle_friend_deal_apply_request()
{
	PROTO_HEAD *head = get_head();
	EXTERN_DATA *extern_data = get_extern_data(head);

	FriendDealApplyRequest *req = friend_deal_apply_request__unpack(NULL, get_data_len(), get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] player[%lu] unpack failed", __FUNCTION__, __LINE__, extern_data->player_id);
		return;
	}

	uint64_t target_id = req->playerid;
	FriendApplyDeal apply_deal = req->deal;
	friend_deal_apply_request__free_unpacked(req, NULL);

	int ret = 0;
	AutoReleaseBatchFriendPlayer arb_friend;
	FriendPlayer *player = NULL;
	std::vector<uint64_t> dealPlayerIds;
	do
	{
		player = get_friend_player(extern_data->player_id);
		if (!player)
		{
			ret = ERROR_ID_SERVER;
			LOG_ERR("[%s:%d] player[%lu] get friend failed", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}
		arb_friend.push_back(player);

		FriendListChangeInfo change_info;
		if (target_id > 0)
		{
			if (apply_deal == FRIEND_APPLY_DEAL__AD_ACCEPT)
			{ //单个同意
				int apply_idx = -1;
				int apply_limit_num = get_apply_limit_num();
				for (int i = 0; i < apply_limit_num; ++i)
				{
					if (player->applys[i] == 0)
					{
						break;
					}
					if (player->applys[i] == target_id)
					{
						apply_idx = i;
						break;
					}
				}

				if (apply_idx < 0)
				{
					ret = ERROR_ID_FRIEND_NOT_IN_LIST;
					LOG_ERR("[%s:%d] player[%lu] not in list, target_id:%lu", __FUNCTION__, __LINE__, extern_data->player_id, target_id);
					break;
				}

				ret = add_contact(player, target_id, change_info, false, false);
				if (ret != 0)
				{
					break;
				}

				del_apply_idx(player, apply_idx);
				dealPlayerIds.push_back(target_id);
			}
			else
			{ //单个拒绝
				del_apply(player, target_id, NULL);
				dealPlayerIds.push_back(target_id);
			}
		}
		else
		{
			if (apply_deal == FRIEND_APPLY_DEAL__AD_ACCEPT)
			{ //全部同意
				int i = 0;
				int apply_limit_num = get_apply_limit_num();
				for (; i < apply_limit_num; ++i)
				{
					if (player->applys[i] == 0)
					{
						break;
					}

					int ret2 = add_contact(player, player->applys[i], change_info, false, false);
					if (ret2 != 0)
					{
						break;
					}

					dealPlayerIds.push_back(player->applys[i]);
				}

				if (i > 0)
				{
					if (i == apply_limit_num)
					{
						memset(&player->applys[0], 0, apply_limit_num * sizeof(uint64_t));
					}
					else
					{
						int last_idx = apply_limit_num - 1;
						memmove(&player->applys[0], &player->applys[i], (last_idx - i) * sizeof(uint64_t));
						memset(&player->applys[last_idx - i], 0, (i + 1) * sizeof(uint64_t));
					}
				}
			}
			else
			{ //全部拒绝
				int apply_limit_num = get_apply_limit_num();
				for (int i = 0; i < apply_limit_num; ++i)
				{
					if (player->applys[i] == 0)
					{
						break;
					}
					dealPlayerIds.push_back(player->applys[i]);
				}
				memset(&player->applys[0], 0, sizeof(uint64_t));
			}
		}

		notify_friend_list_change(player, change_info);
		save_friend_player(player);
	} while (0);

	FriendDealApplyAnswer resp;
	friend_deal_apply_answer__init(&resp);

	resp.result = ret;
	resp.playerid = target_id;
	resp.deal = apply_deal;
	if (dealPlayerIds.size() > 0)
	{
		resp.n_dealplayers = dealPlayerIds.size();
		resp.dealplayers = &dealPlayerIds[0];
	}
	fast_send_msg(&connecter, extern_data, MSG_ID_FRIEND_DEAL_APPLY_ANSWER, friend_deal_apply_answer__pack, resp);
}

void conn_node_friendsrv::handle_friend_recommend_request()
{
	PROTO_HEAD *head = get_head();
	EXTERN_DATA *extern_data = get_extern_data(head);

	std::set<uint64_t> recommendIds;
	std::map<uint64_t, PlayerRedisInfo*> redis_players;
	AutoReleaseBatchRedisPlayer t1;

	PROTO_FRIEND_RECOMMEND *proto = (PROTO_FRIEND_RECOMMEND*)head;
	int ret = 0;
	AutoReleaseBatchFriendPlayer arb_friend;
	FriendPlayer *player = NULL;
	do
	{
		player = get_friend_player(extern_data->player_id);
		if (!player)
		{
			ret = ERROR_ID_SERVER;
			LOG_ERR("[%s:%d] player[%lu] get friend failed", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}
		arb_friend.push_back(player);

		std::vector<uint64_t> playerIds;
		for (uint32_t i = 0; i < proto->player_num; ++i)
		{
			uint64_t tmp_id = proto->player_id[i];
			if (!is_in_contact(player, tmp_id))
			{
				playerIds.push_back(tmp_id);
			}
		}

		while (playerIds.size() > 0 && recommendIds.size() < MAX_FRIEND_RECOMMEND_NUM)
		{
			uint32_t rand_val = rand() % playerIds.size();
			recommendIds.insert(playerIds[rand_val]);
			playerIds.erase(playerIds.begin() + rand_val);
		}
	} while (0);

	FriendRecommendAnswer resp;
	friend_recommend_answer__init(&resp);

	FriendPlayerBriefData recommend_data[MAX_FRIEND_RECOMMEND_NUM];
	FriendPlayerBriefData* recommend_point[MAX_FRIEND_RECOMMEND_NUM];
	AttrData  recommend_attr_data[MAX_FRIEND_RECOMMEND_NUM][MAX_FRIEND_UNIT_ATTR_NUM];
	AttrData* recommend_attr_point[MAX_FRIEND_RECOMMEND_NUM][MAX_FRIEND_UNIT_ATTR_NUM];

	resp.result = ret;

	get_more_redis_player(recommendIds, redis_players, conn_node_friendsrv::server_key, sg_redis_client, t1);
	resp.n_players = 0;
	resp.players = recommend_point;
	for (std::set<uint64_t>::iterator iter = recommendIds.begin(); iter != recommendIds.end(); ++iter)
	{
		uint64_t tmp_id = *iter;
		recommend_point[resp.n_players] = &recommend_data[resp.n_players];
		friend_player_brief_data__init(&recommend_data[resp.n_players]);
		recommend_data[resp.n_players].attrs = recommend_attr_point[resp.n_players];
		recommend_data[resp.n_players].n_attrs = 0;
		for (int j = 0; j < MAX_FRIEND_UNIT_ATTR_NUM; ++j)
		{
			recommend_attr_point[resp.n_players][j] = &recommend_attr_data[resp.n_players][j];
			attr_data__init(&recommend_attr_data[resp.n_players][j]);
		}

		PlayerRedisInfo *redis_player = find_redis_from_map(redis_players, tmp_id);
		if (redis_player)
		{
			set_proto_friend(*redis_player, recommend_data[resp.n_players]);
		}
		recommend_data[resp.n_players].playerid = tmp_id;
		resp.n_players++;
	}

	fast_send_msg(&connecter, extern_data, MSG_ID_FRIEND_RECOMMEND_ANSWER, friend_recommend_answer__pack, resp);

	// for (std::map<uint64_t, PlayerRedisInfo*>::iterator iter = redis_players.begin(); iter != redis_players.end(); ++iter)
	// {
	// 	player_redis_info__free_unpacked(iter->second, NULL);
	// }
}

void conn_node_friendsrv::handle_friend_send_gift_request()
{
	PROTO_HEAD *head = get_head();
	EXTERN_DATA *extern_data = get_extern_data(head);

	FriendSendGiftRequest *req = friend_send_gift_request__unpack(NULL, get_data_len(), get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] player[%lu] unpack failed", __FUNCTION__, __LINE__, extern_data->player_id);
		return;
	}

	uint64_t target_id = req->playerid;
	uint64_t gift_id = req->itemid;
	uint64_t gift_num = req->itemnum;
	friend_send_gift_request__free_unpacked(req, NULL);

	int ret = 0;
	AutoReleaseBatchFriendPlayer arb_friend;
	FriendPlayer *player = NULL;
	do
	{
		player = get_friend_player(extern_data->player_id);
		if (!player)
		{
			ret = ERROR_ID_SERVER;
			LOG_ERR("[%s:%d] player[%lu] get friend failed", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}
		arb_friend.push_back(player);

		if (!player_is_exist(target_id))
		{
			ret = ERROR_ID_FRIEND_ID;
			LOG_ERR("[%s:%d] player[%lu] target not exist, target_id:%lu", __FUNCTION__, __LINE__, extern_data->player_id, target_id);
			break;
		}

		FriendPlayer *target = get_friend_player(target_id);
		if (!target)
		{
			ret = ERROR_ID_FRIEND_ID;
			LOG_ERR("[%s:%d] player[%lu] get target failed, target_id:%lu", __FUNCTION__, __LINE__, extern_data->player_id, target_id);
			break;
		}

		//检查是否互为好友
		int target_idx = get_contact_idx(player, target->player_id);
		if (target_idx < 0)
		{
			ret = 190500198;
			LOG_ERR("[%s:%d] player[%lu] target not friend, target_id:%lu", __FUNCTION__, __LINE__, extern_data->player_id, target_id);
			break;
		}

		if (!is_in_contact(target, player->player_id))
		{
			ret = 190500198;
			LOG_ERR("[%s:%d] player[%lu] not target friend, target_id:%lu", __FUNCTION__, __LINE__, extern_data->player_id, target_id);
			break;
		}

		//检查道具ID和类型
		GiftTable *config = get_config_by_id(gift_id, &friend_gift_config);
		if (!config)
		{
			ret = ERROR_ID_FRIEND_GIFT_ID;
			LOG_ERR("[%s:%d] player[%lu] get config failed, target_id:%lu, gift_id:%lu", __FUNCTION__, __LINE__, extern_data->player_id, target_id, gift_id);
			break;
		}

		try_reset_friend_player(player);
		try_reset_friend_player(target);

		//检查单个玩家赠送数量
		if (player->contacts[target_idx].gift_num >= sg_friend_gift_send_num)
		{
			ret = 190500199;
			break;
		}

		if (player->contacts[target_idx].gift_num + gift_num > sg_friend_gift_send_num)
		{
			ret = 190500201;
			LOG_ERR("[%s:%d] player[%lu] can't send, target_id:%lu, send_num:%u", __FUNCTION__, __LINE__, extern_data->player_id, target_id, player->contacts[target_idx].gift_num);
			break;
		}

		//检查每日最多接受数量
		if (target->gift_accept + gift_num > sg_friend_gift_accept_num)
		{
			ret = 190500200;
			LOG_ERR("[%s:%d] player[%lu] target can't accept, target_id:%lu, accept_num:%u", __FUNCTION__, __LINE__, extern_data->player_id, target_id, target->gift_accept);
			break;
		}

		{ //成功，去game_srv检查道具数量是否足够
			PROTO_COST_FRIEND_GIFT_REQ *cost_req = (PROTO_COST_FRIEND_GIFT_REQ *)get_send_buf(SERVER_PROTO_FRIEND_GIFT_COST_REQUEST, get_seq());
			cost_req->head.len = ENDION_FUNC_4(sizeof(PROTO_COST_FRIEND_GIFT_REQ));
			memset(cost_req->head.data, 0, sizeof(PROTO_COST_FRIEND_GIFT_REQ)-sizeof(PROTO_HEAD));
			cost_req->target_id = target_id;
			cost_req->gift_id = gift_id;
			cost_req->gift_num = gift_num;
			cost_req->item_id = config->TypeId;
			cost_req->currency_type = config->CoinType;
			cost_req->currency_val = config->Value;
			add_extern_data(&cost_req->head, extern_data);
			if (connecter.send_one_msg(&cost_req->head, 1) != (int)ENDION_FUNC_4(cost_req->head.len))
			{
				LOG_ERR("[%s:%d] send to gamesrv failed err[%d]", __FUNCTION__, __LINE__, errno);
				ret = ERROR_ID_SERVER;
			}
		}
	} while (0);

	if (ret != 0)
	{ //失败，返回错误给client
		FriendSendGiftAnswer resp;
		friend_send_gift_answer__init(&resp);

		resp.result = ret;
		resp.playerid = target_id;
		resp.itemid = gift_id;
		resp.itemnum = gift_num;

		fast_send_msg(&connecter, extern_data, MSG_ID_FRIEND_SEND_GIFT_ANSWER, friend_send_gift_answer__pack, resp);
	}

}

void conn_node_friendsrv::handle_friend_gift_cost_answer()
{
	PROTO_HEAD *head = get_head();
	EXTERN_DATA *extern_data = get_extern_data(head);

	PROTO_COST_FRIEND_GIFT_RES *res = (PROTO_COST_FRIEND_GIFT_RES*)buf_head();

	uint64_t target_id = res->target_id;
	uint64_t gift_id = res->gift_id;
	uint64_t gift_num = res->gift_num;

	int ret = res->result;
	AutoReleaseBatchFriendPlayer arb_friend;
	AutoReleaseBatchRedisPlayer arb_redis;

	FriendPlayer *player = NULL;
	bool internal = false;
	do
	{
		if (ret != 0)
		{
			break;
		}
		internal = true;

		player = get_friend_player(extern_data->player_id);
		if (!player)
		{
			ret = ERROR_ID_SERVER;
			LOG_ERR("[%s:%d] player[%lu] get friend failed", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}
		arb_friend.push_back(player);

		FriendPlayer *target = get_friend_player(target_id);
		if (!target)
		{
			ret = ERROR_ID_FRIEND_ID;
			LOG_ERR("[%s:%d] player[%lu] get target failed, target_id:%lu", __FUNCTION__, __LINE__, extern_data->player_id, target_id);
			break;
		}

		int target_idx = get_contact_idx(player, target->player_id);
		if (target_idx < 0)
		{
			ret = 190500198;
			LOG_ERR("[%s:%d] player[%lu] target not friend, target_id:%lu", __FUNCTION__, __LINE__, extern_data->player_id, target_id);
			break;
		}

		int player_idx = get_contact_idx(target, player->player_id);
		if (player_idx < 0)
		{
			ret = 190500198;
			LOG_ERR("[%s:%d] player[%lu] not target friend, target_id:%lu", __FUNCTION__, __LINE__, extern_data->player_id, target_id);
			break;
		}

		GiftTable *config = get_config_by_id(gift_id, &friend_gift_config);
		if (!config)
		{
			ret = ERROR_ID_FRIEND_GIFT_ID;
			LOG_ERR("[%s:%d] player[%lu] get config failed, target_id:%lu, gift_id:%lu", __FUNCTION__, __LINE__, extern_data->player_id, target_id, gift_id);
			break;
		}

		//记录次数
		player->contacts[target_idx].gift_num += gift_num;
		target->gift_accept += gift_num;

		if (res->add_closeness > 0)
		{
			//增加好感度
			player->contacts[target_idx].closeness += res->add_closeness;
			target->contacts[player_idx].closeness += res->add_closeness;
			save_friend_player(player);
			save_friend_player(target);

			notify_friend_closeness_update(player, player->contacts[target_idx]);
			sync_friend_num_to_game_srv(player);
		}
		if (res->item_id > 0)
		{
			//直接送道具给对方
		}

		do
		{
			PlayerRedisInfo *redis_player = get_redis_player(player->player_id, conn_node_friendsrv::server_key, sg_redis_client, arb_redis);
			PlayerRedisInfo *redis_target = get_redis_player(target->player_id, conn_node_friendsrv::server_key, sg_redis_client, arb_redis);

			EXTERN_DATA ext_data;
			ext_data.player_id = target->player_id;

			bool target_online = (redis_target && redis_target->status == 0);
			//通知送礼
			if (target_online)
			{
				FriendSendGiftNotify nty;
				friend_send_gift_notify__init(&nty);

				nty.senderid = player->player_id;
				nty.itemid = gift_id;
				nty.itemnum = gift_num;

				fast_send_msg(&conn_node_friendsrv::connecter, &ext_data, MSG_ID_FRIEND_SEND_GIFT_NOTIFY, friend_send_gift_notify__pack, nty);
			}

			//通知好感度变更
			if (res->add_closeness > 0 && target_online)
			{
				notify_friend_closeness_update(target, target->contacts[player_idx]);
				sync_friend_num_to_game_srv(target);
			}

			//通知收到道具
			if (res->item_id > 0)
			{
				FriendGiftData gift_data;
				friend_gift_data__init(&gift_data);
				ItemData item_data;
				item_data__init(&item_data);
				gift_data.playerid = player->player_id;
				if (redis_player)
					gift_data.playername = redis_player->name;
				gift_data.item = &item_data;
				item_data.id = res->item_id;
				item_data.num = res->gift_num;
				if (target_online)
				{ //目标在线，直接发送
					fast_send_msg(&conn_node_friendsrv::connecter, &ext_data, SERVER_PROTO_FRIEND_ADD_GIFT, friend_gift_data__pack, gift_data);
				}
				else
				{ //目标离线，存离线消息
					add_friend_offline_gift(target->player_id, &gift_data);
				}
			}

			//通知系统消息
			if (redis_player)
			{
				std::vector<char*> args;
				std::stringstream ss;
				std::string sz_id, sz_num;
				ss << config->TypeId;
				ss >> sz_id;
				ss.str("");
				ss.clear();
				ss << gift_num;
				ss >> sz_num;
				args.push_back(redis_player->name);
				args.push_back(const_cast<char*>(sz_num.c_str()));
				args.push_back(const_cast<char*>(sz_id.c_str()));

				SystemNoticeNotify sys;
				system_notice_notify__init(&sys);

				sys.id = 190500195;
				if(res->item_id != 0)
				{
					sys.id = 190500528;
				}
				sys.args = &args[0];
				sys.n_args = args.size();
				sys.targetid = player->player_id;
				sys.has_targetid = true;

				if (target_online)
				{ //在线直接发送
					fast_send_msg(&conn_node_friendsrv::connecter, &ext_data, MSG_ID_SYSTEM_NOTICE_NOTIFY, system_notice_notify__pack, sys);
				}
				else
				{ //离线存库
					add_friend_offline_system(target->player_id, &sys);
				}
			}

		} while (0);

		fast_send_msg_base(&conn_node_friendsrv::connecter, extern_data, SERVER_PROTO_FRIEND_SEND_GIFT_SUCCESS, 0, 0);
	} while (0);

	FriendSendGiftAnswer resp;
	friend_send_gift_answer__init(&resp);

	resp.result = ret;
	resp.playerid = target_id;
	resp.itemid = gift_id;
	resp.itemnum = gift_num;

	fast_send_msg(&conn_node_friendsrv::connecter, extern_data, MSG_ID_FRIEND_SEND_GIFT_ANSWER, friend_send_gift_answer__pack, resp);

	if (ret != 0 && internal)
	{
		PROTO_UNDO_COST *proto = (PROTO_UNDO_COST*)get_send_data();
		uint32_t data_len = sizeof(PROTO_UNDO_COST);
		memset(proto, 0, data_len);
		memcpy(&proto->cost, &res->cost, sizeof(SRV_COST_INFO));
		fast_send_msg_base(&connecter, extern_data, SERVER_PROTO_UNDO_COST, data_len, 0);
	}
}

void conn_node_friendsrv::handle_friend_track_enemy_request()
{
	PROTO_HEAD *head = get_head();
	EXTERN_DATA *extern_data = get_extern_data(head);

	FriendOperateRequest *req = friend_operate_request__unpack(NULL, get_data_len(), get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] player[%lu] unpack failed", __FUNCTION__, __LINE__, extern_data->player_id);
		return;
	}

	uint64_t target_id = req->playerid;
	friend_operate_request__free_unpacked(req, NULL);

	int ret = 0;
	AutoReleaseBatchFriendPlayer arb_friend;
	do
	{
		FriendPlayer *player = get_friend_player(extern_data->player_id);
		if (!player)
		{
			ret = ERROR_ID_SERVER;
			LOG_ERR("[%s:%d] player[%lu] get friend failed", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}
		arb_friend.push_back(player);

		int idx = -1;
		for (int i = 0; i < MAX_FRIEND_ENEMY_NUM; ++i)
		{
			if (player->enemies[i].player_id == 0)
			{
				break;
			}

			if (player->enemies[i].player_id == target_id)
			{
				idx = i;
				break;
			}
		}

		if (idx < 0)
		{
			ret = ERROR_ID_FRIEND_NOT_IN_LIST;
			LOG_ERR("[%s:%d] player[%lu] get enemy[%lu] failed", __FUNCTION__, __LINE__, extern_data->player_id, target_id);
			break;
		}

		EnemyUnit *pEnemy = &player->enemies[idx];
		uint32_t now = time_helper::get_cached_time() / 1000;
		if (pEnemy->track_time > now)
		{
			ret = 190500156;
			LOG_ERR("[%s:%d] player[%lu] enemy[%lu] already in track", __FUNCTION__, __LINE__, extern_data->player_id, target_id);
			break;
		}

		pEnemy->track_time = now + sg_friend_track_time;
		save_friend_player(player);
		
		{
			FRIEND_TRACK_REQUEST *track_req = (FRIEND_TRACK_REQUEST*)get_send_data();
			uint32_t data_len = sizeof(FRIEND_TRACK_REQUEST);
			memset(track_req, 0, data_len);
			track_req->target_id = target_id;
			track_req->cost_item_id = sg_friend_track_item_id;
			track_req->cost_item_num = sg_friend_track_item_num;
			track_req->track_time = pEnemy->track_time;
			fast_send_msg_base(&connecter, extern_data, SERVER_PROTO_FRIEND_TRACK_ENEMY_REQUEST, data_len, 0);
		}
	} while (0);

	if (ret != 0)
	{
		FriendTrackEnemyAnswer resp;
		friend_track_enemy_answer__init(&resp);

		resp.result = ret;
		resp.playerid = target_id;
		resp.tracktime = 0;

		fast_send_msg(&connecter, extern_data, MSG_ID_FRIEND_TRACK_ENEMY_ANSWER, friend_track_enemy_answer__pack, resp);
	}
}

void conn_node_friendsrv::handle_friend_track_enemy_answer()
{
	PROTO_HEAD *head = get_head();
	EXTERN_DATA *extern_data = get_extern_data(head);

	FRIEND_TRACK_ANSWER *res = (FRIEND_TRACK_ANSWER*)get_data();

	uint64_t target_id = res->target_id;

	int ret = 0;
	AutoReleaseBatchFriendPlayer arb_friend;

	EnemyUnit *pEnemy = NULL;
	do
	{
		FriendPlayer *player = get_friend_player(extern_data->player_id);
		if (!player)
		{
			ret = ERROR_ID_SERVER;
			LOG_ERR("[%s:%d] player[%lu] get friend failed", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}
		arb_friend.push_back(player);

		int idx = -1;
		for (int i = 0; i < MAX_FRIEND_ENEMY_NUM; ++i)
		{
			if (player->enemies[i].player_id == 0)
			{
				break;
			}

			if (player->enemies[i].player_id == target_id)
			{
				idx = i;
				break;
			}
		}

		if (idx < 0)
		{
			ret = ERROR_ID_FRIEND_NOT_IN_LIST;
			LOG_ERR("[%s:%d] player[%lu] get enemy[%lu] failed", __FUNCTION__, __LINE__, extern_data->player_id, target_id);
			break;
		}

		pEnemy = &player->enemies[idx];
		if (res->result != 0)
		{
			pEnemy->track_time = 0;
			save_friend_player(player);
		}
	} while (0);

	FriendTrackEnemyAnswer resp;
	friend_track_enemy_answer__init(&resp);

	resp.result = (res->result != 0 ? res->result : ret);
	resp.playerid = target_id;
	resp.tracktime = (pEnemy ? pEnemy->track_time : 0);

	fast_send_msg(&connecter, extern_data, MSG_ID_FRIEND_TRACK_ENEMY_ANSWER, friend_track_enemy_answer__pack, resp);
}

void conn_node_friendsrv::handle_friend_auto_accept_apply_request()
{
	PROTO_HEAD *head = get_head();
	EXTERN_DATA *extern_data = get_extern_data(head);

	int ret = 0;
	AutoReleaseBatchFriendPlayer arb_friend;
	FriendPlayer *player = NULL;
	do
	{
		player = get_friend_player(extern_data->player_id);
		if (!player)
		{
			ret = ERROR_ID_SERVER;
			LOG_ERR("[%s:%d] player[%lu] get friend failed", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}
		arb_friend.push_back(player);

		if (player->auto_accept_apply == 0)
		{
			player->auto_accept_apply = 1;
		}
		else
		{
			player->auto_accept_apply = 0;
		}

		save_friend_player(player);
	} while (0);

	FriendAutoAcceptApplyAnswer resp;
	friend_auto_accept_apply_answer__init(&resp);

	resp.result = ret;
	resp.autoacceptapply = (player != NULL ? player->auto_accept_apply : 0);

	fast_send_msg(&connecter, extern_data, MSG_ID_FRIEND_AUTO_ACCEPT_APPLY_ANSWER, friend_auto_accept_apply_answer__pack, resp);
}

void conn_node_friendsrv::handle_friend_chat_request()
{
	PROTO_HEAD *head = get_head();
	EXTERN_DATA *extern_data = get_extern_data(head);

	Chat *req = chat__unpack(NULL, get_data_len(), get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] player[%lu] unpack failed", __FUNCTION__, __LINE__, extern_data->player_id);
		return;
	}

	int ret = 0;
	AutoReleaseBatchRedisPlayer arb_redis;
	AutoReleaseBatchFriendPlayer arb_friend;
	FriendPlayer *player = NULL;
	do
	{
		player = get_friend_player(extern_data->player_id);
		if (!player)
		{
			ret = ERROR_ID_SERVER;
			LOG_ERR("[%s:%d] player[%lu] get friend failed", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}
		arb_friend.push_back(player);

		if (!req->has_recvplayerid)
		{
			ret = ERROR_ID_FRIEND_CHAT_RECEIVER;
			LOG_ERR("[%s:%d] player[%lu] hasn't receiver", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}

		PlayerRedisInfo *redis_receiver = get_redis_player(req->recvplayerid, conn_node_friendsrv::server_key, sg_redis_client, arb_redis);
		if (!redis_receiver)
		{
			ret = ERROR_ID_FRIEND_CHAT_RECEIVER;
			LOG_ERR("[%s:%d] player[%lu] receiver not exist, receiver_id:%lu", __FUNCTION__, __LINE__, extern_data->player_id, req->recvplayerid);
			break;
		}

		if (is_in_block(player, req->recvplayerid))
		{
			ret = ERROR_ID_FRIEND_IN_BLOCK;
			LOG_ERR("[%s:%d] player[%lu] receiver in block, receiver_id:%lu", __FUNCTION__, __LINE__, extern_data->player_id, req->recvplayerid);
			break;
		}

		//添加上发送时间
		req->sendtime = time_helper::get_cached_time() / 1000;
		req->has_sendtime = true;

		//发送聊天通知
		fast_send_msg(&connecter, extern_data, MSG_ID_CHAT_NOTIFY, chat__pack, *req);

		//添加到最近联系人
		add_recent(player, req->recvplayerid);

		//处理对方
		FriendPlayer *receiver = get_friend_player(req->recvplayerid);
		if (receiver && !is_in_block(receiver, player->player_id))
		{ //如果对方没有把我拉黑，发送消息
			if (redis_receiver->status == 0)
			{ //如果对方在线，直接下发
				EXTERN_DATA ext_data;
				ext_data.player_id = req->recvplayerid;
				fast_send_msg(&connecter, &ext_data, MSG_ID_CHAT_NOTIFY, chat__pack, *req);
				add_recent(receiver, player->player_id);
			}
			else
			{ //如果对方不在线，缓存离线消息
				add_friend_offline_chat(req->recvplayerid, req);
			}
		}
	} while (0);

	CommAnswer resp;
	comm_answer__init(&resp);

	resp.result = ret;
	fast_send_msg(&connecter, extern_data, MSG_ID_CHAT_ANSWER, comm_answer__pack, resp);

	chat__free_unpacked(req, NULL);
}

static int notify_friend_switch_info(FriendPlayer *player, EXTERN_DATA *extern_data)
{
	SettingSwitchNotify resp;
	setting_switch_notify__init(&resp);

	SettingSwitchData switch_data[1];
	SettingSwitchData* switch_data_point[1];

	switch_data_point[0] = &switch_data[0];
	setting_switch_data__init(&switch_data[0]);
	switch_data[0].type = SETTING_SWITCH_TYPE__friend;
	switch_data[0].state = player->apply_switch;

	resp.datas = switch_data_point;
	resp.n_datas = 1;

	fast_send_msg(&conn_node_friendsrv::connecter, extern_data, MSG_ID_SETTING_SWITCH_NOTIFY, setting_switch_notify__pack, resp);

	return 0;
}

void conn_node_friendsrv::handle_player_online_notify()
{
	PROTO_HEAD *head = get_head();
	EXTERN_DATA *extern_data = get_extern_data(head);

	AutoReleaseBatchFriendPlayer arb_friend;
	std::map<uint64_t, PlayerRedisInfo*> redis_players;
	AutoReleaseBatchRedisPlayer t1;
	//玩家上线通知
	do
	{
		FriendPlayer *player = get_friend_player(extern_data->player_id);
		if (!player)
		{
			break;
		}
		arb_friend.push_back(player);

		//发送离线聊天信息
		ChatList *chats = load_friend_chat(extern_data->player_id);
		if (chats)
		{
			for (size_t i = 0; i < chats->n_systems; ++i)
			{
				SystemNoticeNotify *info = chats->systems[i];
				fast_send_msg(&connecter, extern_data, MSG_ID_SYSTEM_NOTICE_NOTIFY, system_notice_notify__pack, *info);
			}
			for (size_t i = 0; i < chats->n_chats; ++i)
			{
				Chat *info = chats->chats[i];
				if (info->sendplayerid > 0)
				{
					add_recent(player, info->sendplayerid);
				}
				fast_send_msg(&connecter, extern_data, MSG_ID_CHAT_NOTIFY, chat__pack, *info);
			}
			for (size_t i = 0; i < chats->n_gifts; ++i)
			{
				FriendGiftData *info = chats->gifts[i];
				fast_send_msg(&connecter, extern_data, SERVER_PROTO_FRIEND_ADD_GIFT, friend_gift_data__pack, *info);
			}
			delete_friend_chat(extern_data->player_id);

			chat_list__free_unpacked(chats, NULL);
		}

		//发送好友申请开关信息
		notify_friend_switch_info(player, extern_data);

		//广播该玩家上线
		std::set<uint64_t> *watchers = get_watch_me_players(player->player_id);
		if (watchers)
		{
			get_more_redis_player(*watchers, redis_players, conn_node_friendsrv::server_key, sg_redis_client, t1);
			std::vector<uint64_t> broadcast_ids;
			for (std::set<uint64_t>::iterator iter = watchers->begin(); iter != watchers->end(); ++iter)
			{
				PlayerRedisInfo *redis_player = find_redis_from_map(redis_players, *iter);
				if (redis_player && redis_player->status == 0)
				{
					broadcast_ids.push_back(*iter);
				}
			}

			if (broadcast_ids.size() > 0)
			{
				FriendUpdateStatusNotify nty;
				friend_update_status_notify__init(&nty);

				nty.playerid = player->player_id;
				nty.offlinetime = 0;

				broadcast_message(MSG_ID_FRIEND_UPDATE_STATUS_NOTIFY, &nty, (pack_func)friend_update_status_notify__pack, broadcast_ids);
			}
		}

		sync_friend_num_to_game_srv(player);
		sync_enemy_to_game_srv(player);
	} while (0);

	// for (std::map<uint64_t, PlayerRedisInfo*>::iterator iter = redis_players.begin(); iter != redis_players.end(); ++iter)
	// {
	// 	player_redis_info__free_unpacked(iter->second, NULL);
	// }

	//发送万妖卡
	handle_list_wanyaoka();
}

void conn_node_friendsrv::handle_player_offline_notify() //玩家下线
{
	PROTO_HEAD *head = get_head();
	EXTERN_DATA *extern_data = get_extern_data(head);

	AutoReleaseBatchFriendPlayer arb_friend;
	AutoReleaseBatchRedisPlayer t1;
	std::map<uint64_t, PlayerRedisInfo*> redis_players;
	//玩家下线通知
	do
	{
		FriendPlayer *player = get_friend_player(extern_data->player_id);
		if (!player)
		{
			break;
		}
		arb_friend.push_back(player);

		//广播该玩家下线
		std::set<uint64_t> *watchers = get_watch_me_players(player->player_id);
		if (watchers)
		{
			get_more_redis_player(*watchers, redis_players, conn_node_friendsrv::server_key, sg_redis_client, t1);
			std::vector<uint64_t> broadcast_ids;
			for (std::set<uint64_t>::iterator iter = watchers->begin(); iter != watchers->end(); ++iter)
			{
				PlayerRedisInfo *redis_player = find_redis_from_map(redis_players, *iter);
				if (redis_player && redis_player->status == 0)
				{
					broadcast_ids.push_back(*iter);
				}
			}

			if (broadcast_ids.size() > 0)
			{
				FriendUpdateStatusNotify nty;
				friend_update_status_notify__init(&nty);

				nty.playerid = player->player_id;
				nty.offlinetime = time_helper::get_cached_time() / 1000;

				broadcast_message(MSG_ID_FRIEND_UPDATE_STATUS_NOTIFY, &nty, (pack_func)friend_update_status_notify__pack, broadcast_ids);
			}
		}
	} while (0);

	// for (std::map<uint64_t, PlayerRedisInfo*>::iterator iter = redis_players.begin(); iter != redis_players.end(); ++iter)
	// {
	// 	player_redis_info__free_unpacked(iter->second, NULL);
	// }
}

void conn_node_friendsrv::handle_friend_add_enemy_request()
{
	PROTO_HEAD *head = get_head();
	EXTERN_DATA *extern_data = get_extern_data(head);

	uint64_t target_id = *((uint64_t*)head->data);

	int ret = 0;
	AutoReleaseBatchFriendPlayer arb_friend;
	do
	{
		FriendPlayer *player = get_friend_player(extern_data->player_id);
		if (!player)
		{
			ret = ERROR_ID_SERVER;
			LOG_ERR("[%s:%d] player[%lu] get friend failed", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}
		arb_friend.push_back(player);

		ret = add_enemy(player, target_id);
		if (ret != 0)
		{
			LOG_ERR("[%s:%d] player[%lu] add enemy failed, target_id:%lu", __FUNCTION__, __LINE__, extern_data->player_id, target_id);
		}
	} while (0);

}

void conn_node_friendsrv::handle_friend_extend_contact_request()
{
	PROTO_HEAD *head = get_head();
	EXTERN_DATA *extern_data = get_extern_data(head);

	int ret = 0;
	AutoReleaseBatchFriendPlayer arb_friend;
	do
	{
		FriendPlayer *player = get_friend_player(extern_data->player_id);
		if (!player)
		{
			ret = ERROR_ID_SERVER;
			LOG_ERR("[%s:%d] player[%lu] get friend failed", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}
		arb_friend.push_back(player);

		if (player->contact_extend > 0)
		{
			ret = ERROR_ID_FRIEND_EXTENDED;
			LOG_ERR("[%s:%d] player[%lu] extended", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}

		player->contact_extend = 1;
		save_friend_player(player);

		fast_send_msg_base(&connecter, extern_data, MSG_ID_FRIEND_CONTACT_EXTEND_NOTIFY, 0, 0);
	} while (0);

	if (ret == 0)
	{ //成功，返回game_srv扣除道具
		int data_len = get_data_len();
		memcpy(get_send_data(), conn_node_base::get_data(), data_len);
		fast_send_msg_base(&connecter, extern_data, SERVER_PROTO_FRIEND_EXTEND_CONTACT_ANSWER, data_len, get_seq());
	}
	else
	{ //失败，返回错误给client
		BagUseAnswer resp;
		bag_use_answer__init(&resp);
		resp.result = ret;
		fast_send_msg(&connecter, extern_data, MSG_ID_BAG_USE_ANSWER, bag_use_answer__pack, resp);
	}
}

void conn_node_friendsrv::handle_friend_cost_answer()
{
	//	PROTO_HEAD *head = get_head();
	//	EXTERN_DATA *extern_data = get_extern_data(head);

	//	PROTO_GUILDSRV_CHECK_AND_COST_RES *res = (PROTO_GUILDSRV_CHECK_AND_COST_RES*)buf_head();
	//	switch(res->statis_id)
	//	{
	//	}
}

void conn_node_friendsrv::handle_friend_turn_switch_request()
{
	PROTO_HEAD *head = get_head();
	EXTERN_DATA *extern_data = get_extern_data(head);

	int ret = 0;
	AutoReleaseBatchFriendPlayer arb_friend;
	FriendPlayer *player = NULL;
	do
	{
		player = get_friend_player(extern_data->player_id);
		if (!player)
		{
			ret = ERROR_ID_SERVER;
			LOG_ERR("[%s:%d] player[%lu] get friend failed", __FUNCTION__, __LINE__, extern_data->player_id);
			break;
		}
		arb_friend.push_back(player);

		if (player->apply_switch == 0)
		{
			player->apply_switch = 1;
		}
		else
		{
			player->apply_switch = 0;
		}

		{
			SettingSwitchNotify resp;
			setting_switch_notify__init(&resp);

			SettingSwitchData switch_data[1];
			SettingSwitchData* switch_data_point[1];

			switch_data_point[0] = &switch_data[0];
			setting_switch_data__init(&switch_data[0]);
			switch_data[0].type = SETTING_SWITCH_TYPE__friend;
			switch_data[0].state = player->apply_switch;

			resp.datas = switch_data_point;
			resp.n_datas = 1;

			fast_send_msg(&conn_node_friendsrv::connecter, extern_data, MSG_ID_SETTING_SWITCH_NOTIFY, setting_switch_notify__pack, resp);
		}

		save_friend_player(player);
	} while (0);

	SettingTurnSwitchAnswer resp;
	setting_turn_switch_answer__init(&resp);

	SettingSwitchData switch_data;
	setting_switch_data__init(&switch_data);
	switch_data.type = SETTING_SWITCH_TYPE__friend;
	if (player)
	{
		switch_data.state = player->apply_switch;
	}

	resp.result = ret;
	resp.data = &switch_data;

	fast_send_msg(&connecter, extern_data, MSG_ID_SETTING_TURN_SWITCH_ANSWER, setting_turn_switch_answer__pack, resp);
}

void conn_node_friendsrv::handle_friend_rename_request() //玩家改名
{
	PROTO_HEAD *head = get_head();
	EXTERN_DATA *extern_data = get_extern_data(head);

	PROTO_FRIEND_SYNC_RENAME *req = (PROTO_FRIEND_SYNC_RENAME*)buf_head();
	AutoReleaseBatchRedisPlayer t1;
	std::map<uint64_t, PlayerRedisInfo*> redis_players;
	do
	{
		//获取到加我为好友的玩家，通知他们
		std::set<uint64_t> *notice_player = get_contact_me_players(extern_data->player_id);
		if (notice_player == NULL)
		{
			break;
		}

		if (get_more_redis_player(*notice_player, redis_players, conn_node_friendsrv::server_key, sg_redis_client, t1) != 0)
		{
			break;
		}

		SystemNoticeNotify sys;
		system_notice_notify__init(&sys);

		std::vector<char *> args;
		args.push_back(req->old_name);
		args.push_back(req->new_name);

		sys.id = 190500193;
		sys.args = &args[0];
		sys.n_args = args.size();
		sys.targetid = extern_data->player_id;
		sys.has_targetid = true;

		std::vector<uint64_t> broadcast_ids;
		for (std::set<uint64_t>::iterator iter = notice_player->begin(); iter != notice_player->end(); ++iter)
		{
			PlayerRedisInfo *redis_player = find_redis_from_map(redis_players, *iter);
			if (redis_player && redis_player->status == 0)
			{
				broadcast_ids.push_back(*iter);
			}
			else
			{
				add_friend_offline_system(*iter, &sys);
			}
		}

		if (broadcast_ids.size() > 0)
		{
			//向所有在线玩家广播
			broadcast_message(MSG_ID_SYSTEM_NOTICE_NOTIFY, &sys, (pack_func)system_notice_notify__pack, broadcast_ids);

			FriendUpdateUnitNotify nty;
			friend_update_unit_notify__init(&nty);

			nty.playerid = extern_data->player_id;
			nty.name = req->new_name;
			broadcast_message(MSG_ID_FRIEND_UPDATE_UNIT_NOTIFY, &nty, (pack_func)friend_update_unit_notify__pack, broadcast_ids);
		}
	} while (0);

	// for (std::map<uint64_t, PlayerRedisInfo*>::iterator iter = redis_players.begin(); iter != redis_players.end(); ++iter)
	// {
	// 	player_redis_info__free_unpacked(iter->second, NULL);
	// }
}

void conn_node_friendsrv::send_to_all_player(uint16_t msg_id, void *data, pack_func func)
{
	PROTO_HEAD* proto_head;
	PROTO_HEAD_CONN_BROADCAST *broadcast_head;
	broadcast_head = (PROTO_HEAD_CONN_BROADCAST *)conn_node_base::global_send_buf;
	broadcast_head->msg_id = ENDION_FUNC_2(SERVER_PROTO_BROADCAST_ALL);
	proto_head = &broadcast_head->proto_head;
	proto_head->msg_id = ENDION_FUNC_2(msg_id);
	proto_head->seq = 0;
	size_t size = func(data, (uint8_t *)proto_head->data);
	proto_head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD)+size);
	broadcast_head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD_CONN_BROADCAST)+size);

	if (connecter.send_one_msg((PROTO_HEAD *)broadcast_head, 1) != (int)(ENDION_FUNC_4(broadcast_head->len))) {
		LOG_ERR("%s %d: send to all failed err[%d]", __FUNCTION__, __LINE__, errno);
	}
}

void conn_node_friendsrv::handle_friend_is_or_no_each_other_request()
{
	PROTO_HEAD *head = get_head();
	EXTERN_DATA *extern_data = get_extern_data(head);

	FriendIsOrNoEachOtherRequest *req = friend_is_or_no_each_other_request__unpack(NULL, get_data_len(), get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] player[%lu] unpack failed", __FUNCTION__, __LINE__, extern_data->player_id);
		return;
	}

	uint64_t target_id = req->player_id;
	friend_is_or_no_each_other_request__free_unpacked(req, NULL);

	AutoReleaseBatchFriendPlayer arb_friend;
	FriendPlayer *player = NULL;
	player = get_friend_player(extern_data->player_id);
	if (!player)
	{
		LOG_ERR("[%s:%d] player[%lu] get friend failed", __FUNCTION__, __LINE__, extern_data->player_id);
		return;
	}
	arb_friend.push_back(player);

	if (!player_is_exist(target_id))
	{
		LOG_ERR("[%s:%d] player[%lu] target not exist, target_id:%lu", __FUNCTION__, __LINE__, extern_data->player_id, target_id);
		return;
	}

	FriendPlayer *target = get_friend_player(target_id);
	if (!target)
	{
		LOG_ERR("[%s:%d] player[%lu] get target failed, target_id:%lu", __FUNCTION__, __LINE__, extern_data->player_id, target_id);
		return;
	}

	int ret = 0;
	do
	{
		//检查是否互为好友
		int target_idx = get_contact_idx(player, target->player_id);
		if (target_idx < 0)
		{
			ret = 190500198;
			LOG_ERR("[%s:%d] player[%lu] target not friend, target_id:%lu", __FUNCTION__, __LINE__, extern_data->player_id, target_id);
			break;
		}

		if (!is_in_contact(target, player->player_id))
		{
			ret = 190500198;
			LOG_ERR("[%s:%d] player[%lu] not target friend, target_id:%lu", __FUNCTION__, __LINE__, extern_data->player_id, target_id);
			break;
		}
	}while(0);
	CommAnswer answer;
	comm_answer__init(&answer);
	answer.result = ret;
	fast_send_msg(&connecter, extern_data, MSG_ID_FRIEND_ID_OR_NO_EACH_OTHER_ANSWER, comm_answer__pack, answer);
}

void conn_node_friendsrv::handle_friend_clean_closeness_request()
{
	PROTO_HEAD *head = get_head();
	EXTERN_DATA *extern_data = get_extern_data(head);

	FRIEND_DUMP_CLOSENESS *res = (FRIEND_DUMP_CLOSENESS*)buf_head();
	uint64_t target_id = res->target_id;
	FriendPlayer *player = get_friend_player(extern_data->player_id);
	if(player == NULL)
	{
		LOG_ERR("[%s:%d] clean player friend closeness fail, get player friend redis data fail player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
		return;
	}
	FriendPlayer *target = get_friend_player(target_id);
	if(player == NULL)
	{
		LOG_ERR("[%s:%d] clean player friend closeness fail, get target friend redis data fail player[%lu]", __FUNCTION__, __LINE__, target_id);
		return;
	}
	int target_idx = get_contact_idx(player, target->player_id);
	if (target_idx < 0)
	{
		LOG_ERR("[%s:%d] player[%lu] target not friend, target_id:%lu", __FUNCTION__, __LINE__, extern_data->player_id, target_id);
		return;
	}

	int player_idx = get_contact_idx(target, player->player_id);
	if (player_idx < 0)
	{
		LOG_ERR("[%s:%d] player[%lu] not target friend, target_id:%lu", __FUNCTION__, __LINE__, extern_data->player_id, target_id);
		return;
	}
	player->contacts[target_idx].closeness = 0;
	target->contacts[player_idx].closeness = 0;

	save_friend_player(player);
	save_friend_player(target);

}
