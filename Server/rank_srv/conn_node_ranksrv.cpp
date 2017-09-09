#include "conn_node_ranksrv.h"
#include "game_event.h"
#include "msgid.h"
#include "error_code.h"
#include "time_helper.h"
#include <vector>
#include <map>
#include <set>
#include "rank.pb-c.h"
#include "player_redis_info.pb-c.h"
#include "redis_util.h"
#include "rank_world_boss.h"
#include "rank_db.pb-c.h"
#include "send_mail.h"
#include "app_data_statis.h"
#include "rank_config.h"
#include "rank_util.h" 

conn_node_ranksrv conn_node_ranksrv::connecter;

CRedisClient sg_redis_client;
uint32_t sg_server_id;
struct event sg_clear_timer_event;
struct timeval sg_clear_timer_val = {3600, 0};	

char sg_player_key[64]; //玩家数据
static std::map<uint32_t, std::string> scm_rank_keys;
static std::map<uint32_t, char *> rank_key_map;


#define MAX_RANK_GET_NUM  100 //前端显示数目
#define MAX_RANK_ADD_NUM  5000 //最多排行数
#define MAX_RANK_ATTR_NUM  10

enum
{
	//等级排行
	RANK_LEVEL_TOTAL = 101,
	RANK_LEVEL_JOB1  = 102,
	RANK_LEVEL_JOB2  = 103,
	RANK_LEVEL_JOB3  = 104,
	RANK_LEVEL_JOB4  = 105,
	RANK_LEVEL_JOB5  = 106,
	//战力排行
	RANK_FC_TOTAL = 201,
	RANK_FC_JOB1  = 202,
	RANK_FC_JOB2  = 203,
	RANK_FC_JOB3  = 204,
	RANK_FC_JOB4  = 205,
	RANK_FC_JOB5  = 206,
	//装备排行
	RANK_EQUIP_TOTAL = 301,
	RANK_EQUIP_JOB1  = 302,
	RANK_EQUIP_JOB2  = 303,
	RANK_EQUIP_JOB3  = 304,
	RANK_EQUIP_JOB4  = 305,
	RANK_EQUIP_JOB5  = 306,
	//财富排行
	RANK_TREASURE_COIN      = 407,
	RANK_TREASURE_GOLD      = 408,
	RANK_TREASURE_BIND_GOLD = 409,
	//八卦排行
	RANK_BAGUA_TOTAL = 501,
	RANK_BAGUA_JOB1  = 502,
	RANK_BAGUA_JOB2  = 503,
	RANK_BAGUA_JOB3  = 504,
	RANK_BAGUA_JOB4  = 505,
	RANK_BAGUA_JOB5  = 506,
	//太极之巅排行
	RANK_PVP3_DIVISION2 = 610,
	RANK_PVP3_DIVISION3 = 611,
	RANK_PVP3_DIVISION4 = 612,
	RANK_PVP3_DIVISION5 = 613,
	RANK_PVP3_DIVISION6 = 614,
	RANK_PVP3_DIVISION7 = 615,
	RANK_PVP3_DIVISION8 = 616,
	RANK_PVP3_DIVISION9 = 617,
};

static void init_rank_key_map()
{
	scm_rank_keys[RANK_LEVEL_TOTAL]                            = "s%u_rank_level_total";
	scm_rank_keys[RANK_LEVEL_JOB1]                             = "s%u_rank_level_job1";
	scm_rank_keys[RANK_LEVEL_JOB2]                             = "s%u_rank_level_job2";
	scm_rank_keys[RANK_LEVEL_JOB3]                             = "s%u_rank_level_job3";
	scm_rank_keys[RANK_LEVEL_JOB4]                             = "s%u_rank_level_job4";
	scm_rank_keys[RANK_LEVEL_JOB5]                             = "s%u_rank_level_job5";
	scm_rank_keys[RANK_FC_TOTAL]                               = "s%u_rank_fc_total";
	scm_rank_keys[RANK_FC_JOB1]                                = "s%u_rank_fc_job1";
	scm_rank_keys[RANK_FC_JOB2]                                = "s%u_rank_fc_job2";
	scm_rank_keys[RANK_FC_JOB3]                                = "s%u_rank_fc_job3";
	scm_rank_keys[RANK_FC_JOB4]                                = "s%u_rank_fc_job4";
	scm_rank_keys[RANK_FC_JOB5]                                = "s%u_rank_fc_job5";
	scm_rank_keys[RANK_EQUIP_TOTAL]                            = "s%u_rank_equip_total";
	scm_rank_keys[RANK_EQUIP_JOB1]                             = "s%u_rank_equip_job1";
	scm_rank_keys[RANK_EQUIP_JOB2]                             = "s%u_rank_equip_job2";
	scm_rank_keys[RANK_EQUIP_JOB3]                             = "s%u_rank_equip_job3";
	scm_rank_keys[RANK_EQUIP_JOB4]                             = "s%u_rank_equip_job4";
	scm_rank_keys[RANK_EQUIP_JOB5]                             = "s%u_rank_equip_job5";
	scm_rank_keys[RANK_TREASURE_COIN]                          = "s%u_rank_treasure_coin";
	scm_rank_keys[RANK_TREASURE_GOLD]                          = "s%u_rank_treasure_gold";
	scm_rank_keys[RANK_TREASURE_BIND_GOLD]                     = "s%u_rank_treasure_bind_gold";
	scm_rank_keys[RANK_BAGUA_TOTAL]                            = "s%u_rank_bagua_total";
	scm_rank_keys[RANK_BAGUA_JOB1]                             = "s%u_rank_bagua_job1";
	scm_rank_keys[RANK_BAGUA_JOB2]                             = "s%u_rank_bagua_job2";
	scm_rank_keys[RANK_BAGUA_JOB3]                             = "s%u_rank_bagua_job3";
	scm_rank_keys[RANK_BAGUA_JOB4]                             = "s%u_rank_bagua_job4";
	scm_rank_keys[RANK_BAGUA_JOB5]                             = "s%u_rank_bagua_job5";
	scm_rank_keys[RANK_PVP3_DIVISION2]                         = "s%u_rank_pvp3_division2";
	scm_rank_keys[RANK_PVP3_DIVISION3]                         = "s%u_rank_pvp3_division3";
	scm_rank_keys[RANK_PVP3_DIVISION4]                         = "s%u_rank_pvp3_division4";
	scm_rank_keys[RANK_PVP3_DIVISION5]                         = "s%u_rank_pvp3_division5";
	scm_rank_keys[RANK_PVP3_DIVISION6]                         = "s%u_rank_pvp3_division6";
	scm_rank_keys[RANK_PVP3_DIVISION7]                         = "s%u_rank_pvp3_division7";
	scm_rank_keys[RANK_PVP3_DIVISION8]                         = "s%u_rank_pvp3_division8";
	scm_rank_keys[RANK_PVP3_DIVISION9]                         = "s%u_rank_pvp3_division9";
}

void init_redis_keys(uint32_t server_id)
{
	sprintf(sg_player_key, "server_%u", server_id);
	sprintf(cur_world_boss_key, "cur_world_boss_server_%u", server_id);
	sprintf(befor_world_boss_key, "befor_world_boss_server_%u", server_id);
	sprintf(tou_mu_world_boss_reward_num, "world_boss_tou_mu_reward_server_%u", server_id);
	sprintf(shou_ling_world_boss_reward_num, "world_boss_shou_ling_reward_server_%u", server_id);
	init_rank_key_map();
	init_world_boss_rank_key_map();
	char rank_key[128];
	for (std::map<uint32_t, std::string>::iterator iter = scm_rank_keys.begin(); iter != scm_rank_keys.end(); ++iter)
	{
		sprintf(rank_key, iter->second.c_str(), server_id);
		iter->second.assign(rank_key);
	}

	for(std::map<uint64_t, std::string>::iterator itr = world_boss_rank_keys.begin(); itr != world_boss_rank_keys.end(); itr++)
	{
			sprintf(rank_key, itr->second.c_str(), server_id, itr->first);
			itr->second.assign(rank_key);
	}
}

static char *get_rank_key(uint32_t rank_type)
{
	std::map<uint32_t, std::string>::iterator iter = scm_rank_keys.find(rank_type);
	if (iter != scm_rank_keys.end())
	{
		return const_cast<char*>(iter->second.c_str());
	}
	return NULL;
}

// class AutoReleaseBatchRedisPlayer
// {
// public:
// 	AutoReleaseBatchRedisPlayer() {}
// 	~AutoReleaseBatchRedisPlayer()
// 	{
// 		for (std::vector<PlayerRedisInfo*>::iterator iter = pointer_vec.begin(); iter != pointer_vec.end(); ++iter)
// 		{
// 			player_redis_info__free_unpacked(*iter, NULL);
// 		}
// 	}

// 	void push_back(PlayerRedisInfo *player)
// 	{
// 		pointer_vec.push_back(player);
// 	}
// private:
// 	std::vector<PlayerRedisInfo *> pointer_vec;
// };

conn_node_ranksrv::conn_node_ranksrv()
{
	max_buf_len = 1024 * 1024;
	buf = (uint8_t *)malloc(max_buf_len + sizeof(EXTERN_DATA));
	assert(buf);
	
	add_msg_handle(SERVER_PROTO_REFRESH_PLAYER_REDIS_INFO, &conn_node_ranksrv::handle_refresh_player_info);
	add_msg_handle(MSG_ID_RANK_INFO_REQUEST, &conn_node_ranksrv::handle_rank_info_request);
	add_msg_handle(SERVER_PROTO_PLAYER_ONLINE_NOTIFY, &conn_node_ranksrv::handle_player_online_notify);
	add_msg_handle(SERVER_PROTO_WORLDBOSS_PLAYER_REDIS_INFO, &conn_node_ranksrv::handle_refresh_player_world_boss_info);
	add_msg_handle(MSG_ID_WORLDBOSS_REAL_RANK_INFO_REQUEST, &conn_node_ranksrv::handle_world_boss_real_rank_info_request);
	add_msg_handle(MSG_ID_WORLDBOSS_ZHUJIEMIAN_INFO_REQUEST, &conn_node_ranksrv::handle_world_boss_zhu_jiemian_info_request);
	add_msg_handle(MSG_ID_WORLDBOSS_LAST_RANK_INFO_REQUEST, &conn_node_ranksrv::handle_world_boss_last_rank_info_request);
	add_msg_handle(SERVER_PROTO_WORLDBOSS_BIRTH_UPDATA_REDIS_INFO, &conn_node_ranksrv::handle_world_timing_birth_updata_info);
}

conn_node_ranksrv::~conn_node_ranksrv()
{
}

void conn_node_ranksrv::add_msg_handle(uint32_t msg_id, handle_func func)
{
	connecter.m_handleMap[msg_id] = func;
}

int conn_node_ranksrv::recv_func(evutil_socket_t fd)
{
	EXTERN_DATA *extern_data;
	PROTO_HEAD *head;	
	for (;;) {
		int ret = get_one_buf();
		if (ret == 0) {
			head = (PROTO_HEAD *)buf_head();
			extern_data = get_extern_data(head);
			int cmd = get_cmd();
			LOG_DEBUG("[%s:%d] cmd: %u, player_id: %lu", __FUNCTION__, __LINE__, cmd, extern_data->player_id);
			if (cmd == SERVER_PROTO_GAMESRV_START)
			{
				LOG_INFO("[%s:%d] game_srv start notify.", __FUNCTION__, __LINE__);
			}

			uint64_t times = time_helper::get_micro_time();
			time_helper::set_cached_time(times / 1000);

			HandleMap::iterator iter = m_handleMap.find(cmd);
			if (iter != m_handleMap.end())
			{
				(this->*(iter->second))(extern_data);
			}
			else
			{
				LOG_ERR("[%s:%d] cmd %u has no handler", __FUNCTION__, __LINE__, cmd);
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

// static PlayerRedisInfo *get_redis_player(uint64_t player_id)
// {
// 	CRedisClient &rc = sg_redis_client;
// 	static uint8_t data_buffer[32 * 1024];
// 	int data_len = 32 * 1024;
// 	char field[64];
// 	sprintf(field, "%lu", player_id);
// 	int ret = rc.hget_bin(sg_player_key, field, (char *)data_buffer, &data_len);
// 	if (ret == 0)
// 	{
// 		return player_redis_info__unpack(NULL, data_len, data_buffer);
// 	}

// 	return NULL;
// }

// int get_more_redis_player(std::set<uint64_t> &player_ids, std::map<uint64_t, PlayerRedisInfo*> &redis_players)
// {
// 	if (player_ids.size() == 0)
// 	{
// 		return 0;
// 	}

// 	std::vector<std::relation_three<uint64_t, char*, int> > player_infos;
// 	for (std::set<uint64_t>::iterator iter = player_ids.begin(); iter != player_ids.end(); ++iter)
// 	{
// 		std::relation_three<uint64_t, char*, int> tmp(*iter, NULL, 0);
// 		player_infos.push_back(tmp);
// 	}

// 	int ret = sg_redis_client.get(sg_player_key, player_infos);
// 	if (ret != 0)
// 	{
// 		LOG_ERR("[%s:%d] hmget failed, ret:%d", __FUNCTION__, __LINE__, ret);
// 		return -1;
// 	}

// 	for (std::vector<std::relation_three<uint64_t, char*, int> >::iterator iter = player_infos.begin(); iter != player_infos.end(); ++iter)
// 	{
// 		PlayerRedisInfo *redis_player = player_redis_info__unpack(NULL, iter->three, (uint8_t*)iter->second);
// 		if (!redis_player)
// 		{
// 			ret = -1;
// 			LOG_ERR("[%s:%d] unpack redis failed, player_id:%lu", __FUNCTION__, __LINE__, iter->first);
// 			break;
// 		}

// 		redis_players[iter->first] = redis_player;
// 	}

// 	for (std::vector<std::relation_three<uint64_t, char*, int> >::iterator iter = player_infos.begin(); iter != player_infos.end(); ++iter)
// 	{
// 		free(iter->second);
// 	}

// 	return ret;
// }

PlayerRedisInfo *find_redis_from_map(std::map<uint64_t, PlayerRedisInfo*> &redis_players, uint64_t player_id)
{
	std::map<uint64_t, PlayerRedisInfo*>::iterator iter = redis_players.find(player_id);
	if (iter != redis_players.end())
	{
		return iter->second;
	}
	return NULL;
}

static void copy_redis_guild_info(PlayerRedisInfo *dest, PlayerRedisInfo *src)
{
	dest->guild_id = src->guild_id;
	int name_len = strlen(src->guild_name);
	if (name_len == 0)
	{
		if (dest->guild_name)
		{
			dest->guild_name[0] = '\0';
		}
	}
	else
	{
		dest->guild_name = (char*)realloc(dest->guild_name, name_len + 1);
		strcpy(dest->guild_name, src->guild_name);
	}
}

void cb_clear_timeout(evutil_socket_t, short, void* /*arg*/)
{
	CRedisClient &rc = sg_redis_client;
	int ret = 0;
	uint32_t rank_num = 0;
	int endNo = 0;
	for (std::map<uint32_t, std::string>::iterator iter = scm_rank_keys.begin(); iter != scm_rank_keys.end(); ++iter)
	{
		const char *rank_key = iter->second.c_str();
		ret = rc.zcard(rank_key, rank_num);
		if (ret != 0)
		{
			continue;
		}

		if (rank_num <= MAX_RANK_ADD_NUM)
		{
			continue;
		}

		endNo = (rank_num - MAX_RANK_ADD_NUM - 1);
		ret = rc.zdel_rank(rank_key, 0, endNo);
		if (ret != 0)
		{
			LOG_ERR("[%s:%d] del rank %s failed, ret:%d", __FUNCTION__, __LINE__, rank_key, ret);
		}
		else
		{
			LOG_DEBUG("[%s:%d] del rank %s, startNo:%d, endNo:%d", __FUNCTION__, __LINE__, rank_key, 0, endNo);
		}
	}

	add_timer(sg_clear_timer_val, &sg_clear_timer_event, NULL);
}

static uint32_t get_player_rank(uint32_t rank_type, uint64_t player_id)
{
	CRedisClient &rc = sg_redis_client;
	uint32_t out_rank = 0xffffffff;
	char *rank_key = get_rank_key(rank_type);
	int ret = rc.zget_rank(rank_key, player_id, out_rank);
	if (ret != 0)
	{
		rc.zcard(rank_key, out_rank); //没有排名的人，相当于最后一名
	}
	out_rank++;

	return out_rank;
}

static void sync_rank_change_to_game_srv(uint32_t rank_type, uint32_t prev_rank, uint32_t post_rank)
{
	if (prev_rank == post_rank)
	{
		return ;
	}

	uint32_t min = 0, max = 0;
	if (post_rank > prev_rank)
	{
		max = post_rank;
		min = prev_rank;
	}
	else
	{
		max = prev_rank;
		min = post_rank;
	}

	CRedisClient &rc = sg_redis_client;
	char *rank_key = get_rank_key(rank_type);
	std::vector<std::pair<uint64_t, uint32_t> > out_vec;
	int ret = rc.zget(rank_key, min - 1, max - 1, out_vec);
	if (ret != 0)
	{
		return;
	}

	PROTO_SYNC_RANK_CHANGE *proto = (PROTO_SYNC_RANK_CHANGE*)conn_node_ranksrv::get_send_buf(SERVER_PROTO_RANK_SYNC_CHANGE, 0);
	proto->type = rank_type;
	proto->num = 0;
	ProtoRankPlayer *pPlayer = proto->changes;
	for (size_t i = 0; i < out_vec.size(); ++i)
	{
		pPlayer[proto->num].player = out_vec[i].first;
		pPlayer[proto->num].lv = min + proto->num;
		proto->num++;
	}

	proto->head.len = ENDION_FUNC_4(sizeof(PROTO_SYNC_RANK_CHANGE) + sizeof(ProtoRankPlayer) * proto->num);

	if (conn_node_ranksrv::connecter.send_one_msg(&proto->head, 1) != (int)ENDION_FUNC_4(proto->head.len))
	{
		LOG_ERR("[%s:%d] send to game_srv failed err[%d]", __FUNCTION__, __LINE__, errno);
		return ;
	}
}

static void update_player_rank_score(uint32_t rank_type, uint64_t player_id, uint32_t score_old, uint32_t score_new)
{
	char *rank_key = NULL;
	uint32_t post_rank = 0xffffffff;
	uint32_t prev_rank = 0xffffffff;
	CRedisClient &rc = sg_redis_client;
	rank_key = get_rank_key(rank_type);
	prev_rank = get_player_rank(rank_type, player_id);
	int ret = rc.zset(rank_key, player_id, score_new);
	if (ret != 0)
	{
		LOG_ERR("[%s:%d] update %s %lu failed, score_new:%u, score_old:%u", __FUNCTION__, __LINE__, rank_key, player_id, score_new, score_old);
	}
	else
	{
		post_rank = get_player_rank(rank_type, player_id);
		sync_rank_change_to_game_srv(rank_type, prev_rank, post_rank);
	}
}

int conn_node_ranksrv::handle_refresh_player_info(EXTERN_DATA *extern_data)
{
	AutoReleaseBatchRedisPlayer arb_redis;
	int proto_data_len = get_data_len();
	uint8_t *proto_data = get_data();
	uint32_t refresh_type = *((uint32_t*)proto_data);
	PlayerRedisInfo *req = player_redis_info__unpack(NULL, proto_data_len - sizeof(uint32_t), proto_data + sizeof(uint32_t));
	if (!req)
	{
		LOG_ERR("[%s:%d] player[%lu] unpack req failed, refresh_type:%u", __FUNCTION__, __LINE__, extern_data->player_id, refresh_type);
		return -1;
	}
	arb_redis.push_back(req);

	LOG_INFO("[%s:%d] player[%lu], refresh_type:%u, status:%u", __FUNCTION__, __LINE__, extern_data->player_id, refresh_type, req->status);

	CRedisClient &rc = sg_redis_client;
	char field[128];
	sprintf(field, "%lu", extern_data->player_id);
	uint32_t rank_type = 0;
	char *rank_key = NULL;
	int ret = 0;
	uint64_t player_id = extern_data->player_id;
	uint32_t post_rank = 0xffffffff;
	uint32_t prev_rank = 0xffffffff;

	PlayerRedisInfo *redis_player = get_redis_player(extern_data->player_id, sg_player_key, sg_redis_client, arb_redis);
	if (redis_player)
	{
		LOG_DEBUG("[%s:%d] player[%lu] redis, refresh_type:%u, status:%u", __FUNCTION__, __LINE__, extern_data->player_id, refresh_type, redis_player->status);
	}
	PlayerRedisInfo *save_player = NULL;

	if (refresh_type == 1) //从game_srv同步的消息
	{
		uint32_t job = req->job;
		uint32_t level = req->lv;
		uint32_t fc_total = req->fighting_capacity;
		uint32_t fc_equip = req->equip_fc;
		uint32_t fc_bagua = req->bagua_fc;
		uint32_t coin = req->coin;
		uint32_t gold = req->gold;
		uint32_t gold_bind = req->bind_gold;
		uint32_t pvp3_division = req->pvp_division_3;
		uint32_t pvp3_score = req->pvp_score_3;
		
		uint32_t old_level = 0;
		uint32_t old_fc_total = 0;
		uint32_t old_fc_equip = 0;
		uint32_t old_fc_bagua = 0;
		uint32_t old_coin = 0;
		uint32_t old_gold = 0;
		uint32_t old_gold_bind = 0;
		uint32_t old_pvp3_division = 0;
		uint32_t old_pvp3_score = 0;

		if (redis_player)
		{
			copy_redis_guild_info(req, redis_player);
			save_player = req;

			old_level = redis_player->lv;
			old_fc_total = redis_player->fighting_capacity;
			old_fc_equip = redis_player->equip_fc;
			old_fc_bagua = redis_player->bagua_fc;
			old_coin = redis_player->coin;
			old_gold = redis_player->gold;
			old_gold_bind = redis_player->bind_gold;
			old_pvp3_division = redis_player->pvp_division_3;
			old_pvp3_score = redis_player->pvp_score_3;
		}
		else
		{
			save_player = req;
		}

		if (level != old_level)
		{
			rank_type = RANK_LEVEL_TOTAL;
			update_player_rank_score(rank_type, player_id, old_level, level);

			rank_type = RANK_LEVEL_TOTAL + job;
			update_player_rank_score(rank_type, player_id, old_level, level);
		}

		if (fc_total != old_fc_total)
		{
			rank_type = RANK_FC_TOTAL;
			update_player_rank_score(rank_type, player_id, old_fc_total, fc_total);

			rank_type = RANK_FC_TOTAL + job;
			update_player_rank_score(rank_type, player_id, old_fc_total, fc_total);

			if (req->zhenying != 0)
			{
				PROTO_ZHENYIN_CHANGE_POWER *toFriend = (PROTO_ZHENYIN_CHANGE_POWER *)get_send_data();
				toFriend->power = fc_total - old_fc_total;
				toFriend->zhen_ying = req->zhenying;
				fast_send_msg_base(&connecter, extern_data, SERVER_PROTO_ZHENYING_CHANGE_POWER_REQUEST, sizeof(toFriend), 0);
			}
		}

		if (fc_equip != old_fc_equip)
		{
			rank_type = RANK_EQUIP_TOTAL;
			update_player_rank_score(rank_type, player_id, old_fc_equip, fc_equip);

			rank_type = RANK_EQUIP_TOTAL + job;
			update_player_rank_score(rank_type, player_id, old_fc_equip, fc_equip);
		}

		if (fc_bagua != old_fc_bagua)
		{
			rank_type = RANK_BAGUA_TOTAL;
			update_player_rank_score(rank_type, player_id, old_fc_bagua, fc_bagua);

			rank_type = RANK_BAGUA_TOTAL + job;
			update_player_rank_score(rank_type, player_id, old_fc_bagua, fc_bagua);
		}

		if (coin != old_coin)
		{
			rank_type = RANK_TREASURE_COIN;
			update_player_rank_score(rank_type, player_id, old_coin, coin);
		}

		if (gold != old_gold)
		{
			rank_type = RANK_TREASURE_GOLD;
			update_player_rank_score(rank_type, player_id, old_gold, gold);
		}

		if (gold_bind != old_gold_bind)
		{
			rank_type = RANK_TREASURE_BIND_GOLD;
			update_player_rank_score(rank_type, player_id, old_gold_bind, gold_bind);
		}

		if (pvp3_score != old_pvp3_score)
		{
			if (pvp3_division > 1)
			{
				rank_type = RANK_PVP3_DIVISION2 + pvp3_division - 2;
				update_player_rank_score(rank_type, player_id, old_pvp3_score, pvp3_score);
			}

			if (pvp3_division != old_pvp3_division)
			{
				rank_type = RANK_PVP3_DIVISION2 + old_pvp3_division - 2;
				rank_key = get_rank_key(rank_type);
				prev_rank = get_player_rank(rank_type, player_id);
				std::vector<uint64_t> dels;
				dels.push_back(player_id);
				ret = rc.zdel(rank_key, dels);
				if (ret != 0)
				{
					LOG_ERR("[%s:%d] del %s %lu failed, score:%u, old_score:%u", __FUNCTION__, __LINE__, rank_key, player_id, pvp3_score, old_pvp3_score);
				}
				else
				{
					post_rank = get_player_rank(rank_type, player_id);
					sync_rank_change_to_game_srv(rank_type, prev_rank, post_rank);
				}
			}
		}
	}
	else if (refresh_type == 2) //从guild_srv同步的消息，只更新帮会信息
	{
		if (redis_player)
		{
			copy_redis_guild_info(redis_player, req);
			save_player = redis_player;
		}
		else
		{
			save_player = req;
		}
	}

	//把数据打包保存
	if (save_player)
	{
		static uint8_t data_buffer[128 * 1024];
		do
		{
			size_t data_len = player_redis_info__pack(save_player, data_buffer);
			if (data_len == (size_t)-1)
			{
				LOG_ERR("[%s:%d] pack redis player failed, player[%lu]", __FUNCTION__, __LINE__, extern_data->player_id);
				break;
			}

			sprintf(field, "%lu", extern_data->player_id);
			int ret = rc.hset_bin(sg_player_key, field, (const char *)data_buffer, (int)data_len);
			if (ret < 0)
			{
				LOG_ERR("[%s:%d] set redis player failed, player[%lu] ret = %d", __FUNCTION__, __LINE__, extern_data->player_id, ret);
				break;
			}
			LOG_DEBUG("[%s:%d] save player[%lu] len[%d] ret = %d, refresh_type:%u, status:%u", __FUNCTION__, __LINE__, extern_data->player_id, (int)data_len, ret, refresh_type, save_player->status);
		} while(0);
	}

	return 0;
}

static void fill_rank_player(PlayerRedisInfo *redis, RankPlayerData *rank)
{
	rank->baseinfo->name = redis->name;

	rank->baseinfo->n_attrs = 0;
	rank->baseinfo->attrs[rank->baseinfo->n_attrs]->id = PLAYER_ATTR_LEVEL;
	rank->baseinfo->attrs[rank->baseinfo->n_attrs]->val = redis->lv;
	rank->baseinfo->n_attrs++;
	rank->baseinfo->attrs[rank->baseinfo->n_attrs]->id = PLAYER_ATTR_JOB;
	rank->baseinfo->attrs[rank->baseinfo->n_attrs]->val = redis->job;
	rank->baseinfo->n_attrs++;
	rank->baseinfo->attrs[rank->baseinfo->n_attrs]->id = PLAYER_ATTR_HEAD;
	rank->baseinfo->attrs[rank->baseinfo->n_attrs]->val = redis->head_icon;
	rank->baseinfo->n_attrs++;
	rank->baseinfo->attrs[rank->baseinfo->n_attrs]->id = PLAYER_ATTR_CLOTHES;
	rank->baseinfo->attrs[rank->baseinfo->n_attrs]->val = redis->clothes;
	rank->baseinfo->n_attrs++;
	rank->baseinfo->attrs[rank->baseinfo->n_attrs]->id = PLAYER_ATTR_CLOTHES_COLOR_UP;
	rank->baseinfo->attrs[rank->baseinfo->n_attrs]->val = redis->clothes_color_up;
	rank->baseinfo->n_attrs++;
	rank->baseinfo->attrs[rank->baseinfo->n_attrs]->id = PLAYER_ATTR_CLOTHES_COLOR_DOWN;
	rank->baseinfo->attrs[rank->baseinfo->n_attrs]->val = redis->clothes_color_down;
	rank->baseinfo->n_attrs++;
	rank->baseinfo->attrs[rank->baseinfo->n_attrs]->id = PLAYER_ATTR_HAT;
	rank->baseinfo->attrs[rank->baseinfo->n_attrs]->val = redis->hat;
	rank->baseinfo->n_attrs++;
	rank->baseinfo->attrs[rank->baseinfo->n_attrs]->id = PLAYER_ATTR_HAT_COLOR;
	rank->baseinfo->attrs[rank->baseinfo->n_attrs]->val = redis->hat_color;
	rank->baseinfo->n_attrs++;
	rank->baseinfo->attrs[rank->baseinfo->n_attrs]->id = PLAYER_ATTR_WEAPON;
	rank->baseinfo->attrs[rank->baseinfo->n_attrs]->val = redis->weapon;
	rank->baseinfo->n_attrs++;

	rank->baseinfo->tags = redis->tags;
	rank->baseinfo->n_tags = redis->n_tags;
	rank->baseinfo->textintro = redis->textintro;

	rank->guildid = redis->guild_id;
	rank->guildname = redis->guild_name;
}

int conn_node_ranksrv::handle_rank_info_request(EXTERN_DATA *extern_data)
{
	RankInfoRequest *req = rank_info_request__unpack(NULL, get_data_len(), get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] player[%lu] unpack failed", __FUNCTION__, __LINE__, extern_data->player_id);
		return -1;
	}

	uint64_t rank_type = req->type;
	rank_info_request__free_unpacked(req, NULL);

	int ret = 0;
	std::vector<std::pair<uint64_t, uint32_t> > rank_info;
	AutoReleaseBatchRedisPlayer t1;		
	std::map<uint64_t, PlayerRedisInfo *> redis_players;
	uint32_t my_rank = 0;
	do
	{
		char *rank_key = get_rank_key(rank_type);
		if (rank_key == NULL)
		{
			ret = ERROR_ID_RANK_TYPE;
			LOG_ERR("[%s:%d] player[%lu] rank type, rank_type:%lu", __FUNCTION__, __LINE__, extern_data->player_id, rank_type);
			break;
		}

		int ret2 = sg_redis_client.zget(rank_key, 0, 99, rank_info);
		if (ret2 != 0)
		{
			ret = ERROR_ID_RANK_REDIS;
			LOG_ERR("[%s:%d] player[%lu] get rank failed, rank_type:%lu, rank_key:%s", __FUNCTION__, __LINE__, extern_data->player_id, rank_type, rank_key);
			break;
		}

		std::set<uint64_t> playerIds;
		for (size_t i = 0; i < rank_info.size(); ++i)
		{
			playerIds.insert(rank_info[i].first);
			if (rank_info[i].first == extern_data->player_id)
			{
				my_rank = i + 1;
			}
		}

		if (get_more_redis_player(playerIds, redis_players, sg_player_key, sg_redis_client, t1) != 0)
		{
			ret = ERROR_ID_RANK_REDIS;
			LOG_ERR("[%s:%d] player[%lu] get player failed, rank_type:%lu", __FUNCTION__, __LINE__, extern_data->player_id, rank_type);
			break;
		}

		if (my_rank == 0)
		{
			int ret2 = sg_redis_client.zget_rank(rank_key, extern_data->player_id, my_rank);
			if (ret2 == 0)
			{
				if (my_rank >= MAX_RANK_ADD_NUM)
				{
					my_rank = 0;
				}
				else
				{
					my_rank++;
				}
			}
		}
	} while(0);

	RankInfoAnswer resp;
	rank_info_answer__init(&resp);

	RankPlayerData  rank_data[MAX_RANK_GET_NUM];
	RankPlayerData* rank_point[MAX_RANK_GET_NUM];
	PlayerBaseData  base_data[MAX_RANK_GET_NUM];
	AttrData  attr_data[MAX_RANK_GET_NUM][MAX_RANK_ATTR_NUM];
	AttrData* attr_point[MAX_RANK_GET_NUM][MAX_RANK_ATTR_NUM];

	resp.result = ret;
	resp.myrank = my_rank;
	resp.infos = rank_point;
	resp.n_infos = 0;
	for (size_t i = 0; i < rank_info.size(); ++i)
	{
		uint64_t player_id = rank_info[i].first;
		uint32_t score = rank_info[i].second;

		rank_point[resp.n_infos] = &rank_data[resp.n_infos];
		rank_player_data__init(&rank_data[resp.n_infos]);

		player_base_data__init(&base_data[resp.n_infos]);
		rank_data[resp.n_infos].baseinfo = &base_data[resp.n_infos];

		rank_data[resp.n_infos].baseinfo->attrs = attr_point[resp.n_infos];
		rank_data[resp.n_infos].baseinfo->n_attrs = 0;
		for (int j = 0; j < MAX_RANK_ATTR_NUM; ++j)
		{
			attr_point[resp.n_infos][j] = &attr_data[resp.n_infos][j];
			attr_data__init(&attr_data[resp.n_infos][j]);
		}

		PlayerRedisInfo *redis_player = find_redis_from_map(redis_players, player_id);
		if (redis_player)
		{
			fill_rank_player(redis_player, &rank_data[resp.n_infos]);
		}

		rank_data[resp.n_infos].baseinfo->playerid = player_id;
		rank_data[resp.n_infos].ranknum = i + 1;
		rank_data[resp.n_infos].score = score;
		resp.n_infos++;
	}

	fast_send_msg(&connecter, extern_data, MSG_ID_RANK_INFO_ANSWER, rank_info_answer__pack, resp);

	// for (std::map<uint64_t, PlayerRedisInfo*>::iterator iter = redis_players.begin(); iter != redis_players.end(); ++iter)
	// {
	// 	player_redis_info__free_unpacked(iter->second, NULL);
	// }
	return 0;
}

int conn_node_ranksrv::handle_player_online_notify(EXTERN_DATA *extern_data)
{
	int ret = 0;
	do
	{
		uint32_t out_rank = 0xffffffff;
		PROTO_SYNC_RANK *proto = (PROTO_SYNC_RANK*)get_send_data();
		memset(proto->ranks, 0, sizeof(proto->ranks));
		int i = 0;
		for (std::map<uint32_t, std::string>::iterator iter = scm_rank_keys.begin(); iter != scm_rank_keys.end() && i < MAX_RANK_TYPE; ++iter)
		{
			out_rank = 0xffffffff;
			ret = sg_redis_client.zget_rank(iter->second.c_str(), extern_data->player_id, out_rank);
			if (ret == 0)
			{
				out_rank++;
			}
			proto->ranks[i].type = iter->first;
			proto->ranks[i].rank = out_rank;
			i++;
		}

		fast_send_msg_base(&connecter, extern_data, SERVER_PROTO_RANK_SYNC_RANK, sizeof(PROTO_SYNC_RANK), 0);
	} while(0);
	
	return 0;
}

int conn_node_ranksrv::handle_refresh_player_world_boss_info(EXTERN_DATA *extern_data)
{
	PlayerWorldBossRedisinfo *req = player_world_boss_redisinfo__unpack(NULL, get_data_len(), get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] player[%lu] unpack req failed, refresh_type:%u", __FUNCTION__, __LINE__, extern_data->player_id);
		return -1;
	}

	char name[MAX_PLAYER_NAME_LEN];
	strcpy(name, req->name);
	uint64_t boss_id = req->boss_id;
	uint32_t score = req->score;
	uint32_t cur_hp = req->cur_hp;
	uint32_t max_hp = req->max_hp;
	uint64_t player_id = extern_data->player_id;
	player_world_boss_redisinfo__free_unpacked(req, NULL);


	std::set<uint64_t>::iterator itr = world_boss_id.find(boss_id);
	if(itr == world_boss_id.end())
	{
		LOG_ERR("[%s:%d]更新世界boss数据失败，无对应的世界boss,bossid[%u]", __FUNCTION__, __LINE__, boss_id);
		return -2;
	}

	CRedisClient &rc = sg_redis_client;
	char field[128];
	int ret;
	char *rank_key = NULL;
	char *befor_rank_key = NULL;
	uint32_t old_score = 0;
	uint32_t out_rank = 0xffffffff;
	uint32_t old_rank = MAX_WORLD_BOSS_REALTINE_RANK_NUM +1;
	uint32_t new_rank = MAX_WORLD_BOSS_REALTINE_RANK_NUM +1;
	char max_score_name[MAX_PLAYER_NAME_LEN] = {0};//上轮最高积分玩家姓名
	uint64_t max_score_player_id = 0; //上轮最高积分玩家uuid
	
	rank_key = get_world_boss_rank_key(boss_id);
	befor_rank_key = get_world_boss_rank_key(boss_id*10);
	if(rank_key == NULL || befor_rank_key == NULL)
	{
		LOG_ERR("[%s:%d]世界boss被击，跟新redis数据失败，redis key 获取出错，bossid[%lu]", __FUNCTION__, __LINE__, boss_id);
		return -3;
	}


	//积分大于0更新排行榜数据(积分大于0说明是有效积分，即给boss制造伤害的玩家的等级在限制范围内)
	if(score >0)
	{	
		//获取之前的排名
		out_rank = 0xffffffff;
		ret = rc.zget_rank(rank_key, player_id, out_rank);
		if (ret == 0)
		{
			out_rank++;
			old_rank = out_rank;
		}

		//更新排行榜内的积分
		ret = rc.zget_score(rank_key, player_id, old_score);
		if(ret == 0)
		{
			score += old_score;
		}
		ret = rc.zset(rank_key, player_id, score);
		if (ret != 0)
		{
			LOG_ERR("[%s:%d] update %s %lu failed, score:%u, old_score:%u", __FUNCTION__, __LINE__, rank_key, player_id, score, old_score);
		}

		//获取跟新后的排名信息
		out_rank = 0xffffffff;
		ret = rc.zget_rank(rank_key, player_id, out_rank);
		if (ret == 0)
		{
			out_rank++;
			new_rank = out_rank;
		}
		//世界boss被击受伤，排行榜内前MAX_WORLD_BOSS_REALTINE_RANK_NUM名玩家数据变化，实时广播排行榜
		if(new_rank <= MAX_WORLD_BOSS_REALTINE_RANK_NUM || old_rank <= MAX_WORLD_BOSS_REALTINE_RANK_NUM)
		{
			broadcast_world_boss_rank_info(boss_id);
		}
		//更新当前对boss制造伤害的玩家的信息
		updata_player_cur_world_boss_info( boss_id, extern_data);
		//world_boss_provide_reward(boss_id);
	}
	


	if(cur_hp <= 0)
	{
		//世界boss死了将当前排行榜数据替换上轮排行榜内
		do{
			ret = rc.zdel_rank(befor_rank_key, 0, -1);
			if(ret != 0)
			{
				LOG_ERR("[%s:%d]将本轮排行信息跟新到上轮时，清除上轮榜单信息失败[%lu]", __FUNCTION__, __LINE__, boss_id);
				break;
			}
			std::vector<std::pair<uint64_t, uint32_t> > cur_world_boss_rank_info;
			rc.zget(rank_key, 0, -1, cur_world_boss_rank_info);
			for(size_t i =0; i < cur_world_boss_rank_info.size(); ++i)
			{
				ret = rc.zset(befor_rank_key, cur_world_boss_rank_info[i].first, cur_world_boss_rank_info[i].second);
			}
			//ret = rc.zdel_rank(rank_key, 0, -1);
			//if(ret != 0)
			//{
			//	LOG_ERR("[%s:%d]将本轮排行信息跟新到上轮时，清除本轮榜单信息失败[%lu]", __FUNCTION__, __LINE__, boss_id)	
			//	break;
			//}
		}while(0);

		//世界boss死了，将数据存入上轮的reids
		BeforWorldBossRedisinfo befor_boss_info;
		befor_world_boss_redisinfo__init(&befor_boss_info);
		befor_boss_info.boss_id = boss_id;
		befor_boss_info.player_id = player_id;
		befor_boss_info.name = name;
		//获取上轮最高积分玩家的数据
		std::vector<std::pair<uint64_t, uint32_t> > befor_world_boss_rank_info;
		rc.zget(rank_key, 0, 0, befor_world_boss_rank_info);
		if(befor_world_boss_rank_info.size() == 1)
		{
			max_score_player_id = befor_world_boss_rank_info[0].first;
		}

		AutoReleaseRedisPlayer p1;
		PlayerRedisInfo *max_score_player_redis = get_redis_player(max_score_player_id, sg_player_key, sg_redis_client, p1);
		if(max_score_player_redis)
		{
			strcpy(max_score_name, max_score_player_redis->name);
			befor_boss_info.max_score_name = max_score_name;
			befor_boss_info.max_score_player_id = max_score_player_id;
		}
		static uint8_t data_buffer[128 * 1024];
		sprintf(field, "%lu", boss_id);
		do
		{
			size_t data_len = befor_world_boss_redisinfo__pack(&befor_boss_info, data_buffer);
			if (data_len == (size_t)-1)
			{
				LOG_ERR("[%s:%d] pack redis world_boss_info failed, bossid[%lu]", __FUNCTION__, __LINE__, boss_id);
				break;
			}

			 ret = rc.hset_bin(befor_world_boss_key, field, (const char *)data_buffer, (int)data_len);
			if (ret < 0)
			{
				LOG_ERR("[%s:%d] set befor world boss failed, bossid[%lu] ret = %d", __FUNCTION__, __LINE__, boss_id, ret);
				break;
			}
		} while(0);
		//将其在当前redis里面的数据清除
		ret = rc.hdel(cur_world_boss_key, field);
		if(ret < 0)
		{
			LOG_ERR("[%s:%d] del cur  world boss failed, bossid[%lu] ret = %d", __FUNCTION__, __LINE__, boss_id, ret)
		}
		//世界boss死了发奖
		world_boss_provide_rank_reward(boss_id);
		world_boss_provide_kill_reward(boss_id);
	}
	else
	{
		CurWorldBossRedisinfo cur_boss_info;
		cur_world_boss_redisinfo__init(&cur_boss_info);
		//cur_boss_info.player_id = player_id;
		//cur_boss_info.name = name;
		cur_boss_info.boss_id = boss_id;
		cur_boss_info.max_hp = max_hp;
		cur_boss_info.cur_hp = cur_hp;
		static uint8_t data_buffer[128 * 1024];
		do
		{
			size_t data_len = cur_world_boss_redisinfo__pack(&cur_boss_info, data_buffer);
			if (data_len == (size_t)-1)
			{
				LOG_ERR("[%s:%d] pack redis world_boss_info failed, bossid[%lu]", __FUNCTION__, __LINE__, boss_id);
				break;
			}

			sprintf(field, "%lu", boss_id);
			ret = rc.hset_bin(cur_world_boss_key, field, (const char *)data_buffer, (int)data_len);
			if (ret < 0)
			{
				LOG_ERR("[%s:%d] set cur word boss failed, bossid[%lu] ret = %d", __FUNCTION__, __LINE__, boss_id, ret);
				break;
			}
		} while(0);
	
	}

	return 0;
}

int conn_node_ranksrv::handle_world_boss_real_rank_info_request(EXTERN_DATA *extern_data)
{
	RankWorldBossRealInfoRequest *req = rank_world_boss_real_info_request__unpack(NULL, get_data_len(), get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] player[%lu] unpack failed", __FUNCTION__, __LINE__, extern_data->player_id);
		return -1;
	}
	
	uint64_t boss_id = req->bossid;
	rank_world_boss_real_info_request__free_unpacked(req, NULL);


	int ret = 0;
	CRedisClient &rc = sg_redis_client;
	char *rank_key = NULL;
	std::vector<std::pair<uint64_t, uint32_t> > cur_world_boss_rank_info;
	uint64_t player_id = extern_data->player_id;
	uint32_t out_rank = 0xffffffff;
	uint32_t my_rank = 0;
	uint32_t my_score = 0;


	RankWorldBossRealInfoAnswer answer;
	rank_world_boss_real_info_answer__init(&answer);
	RankWorldBossInfo rank_info[MAX_WORLD_BOSS_REALTINE_RANK_NUM];
	RankWorldBossInfo *rank_info_point[MAX_WORLD_BOSS_REALTINE_RANK_NUM];
	RankWorldBossPlayerInfo my_info;
	int endNO = MAX_WORLD_BOSS_REALTINE_RANK_NUM -1;
	char name[MAX_WORLD_BOSS_REALTINE_RANK_NUM][MAX_PLAYER_NAME_LEN];

	answer.bossid = boss_id;
	answer.infos = rank_info_point;
	answer.n_infos = 0;
	do
	{
		rank_key = get_world_boss_rank_key(boss_id);
		if( rank_key == NULL)
		{
			ret = 1;
			break;
		}

		ret = rc.zget(rank_key, 0, endNO, cur_world_boss_rank_info);
		AutoReleaseBatchRedisPlayer t1;	
		std::map<uint64_t, PlayerRedisInfo*> redis_players;
		std::set<uint64_t> notice_player;//榜单内玩家唯一id
		for(uint32_t i = 0;  i < cur_world_boss_rank_info.size(); i++)
		{
			notice_player.insert(cur_world_boss_rank_info[i].first);
		}
		
		get_more_redis_player(notice_player ,redis_players, sg_player_key, sg_redis_client, t1);
		if(ret == 0 && cur_world_boss_rank_info.size() >0)
		{		
			for(uint32_t i = 0; i <MAX_WORLD_BOSS_REALTINE_RANK_NUM && i < cur_world_boss_rank_info.size(); i++)
			{
				rank_info_point[answer.n_infos] = &rank_info[answer.n_infos];
				rank_world_boss_info__init(&rank_info[answer.n_infos]);
				rank_info[answer.n_infos].player_id = cur_world_boss_rank_info[i].first;
				rank_info[answer.n_infos].score = cur_world_boss_rank_info[i].second;
				PlayerRedisInfo *redis_player = find_redis_from_map(redis_players,rank_info[answer.n_infos].player_id);
				memset(name[answer.n_infos], 0, MAX_PLAYER_NAME_LEN);
				if(redis_player)
				{
					strcpy(name[answer.n_infos], redis_player->name);
					rank_info[answer.n_infos].name = name[answer.n_infos];
				}
				rank_info[answer.n_infos].ranknum = i + 1;

				answer.n_infos++;
			}
		}

		//获取玩家自己的积分和排名信息
		out_rank = 0xffffffff;
		ret = rc.zget_rank(rank_key, player_id, out_rank);
		if (ret == 0)
		{
			out_rank++;
			my_rank = out_rank;
		}
		ret = rc.zget_score(rank_key, player_id, my_score);	

		answer.my_rankinfo = &my_info;
		rank_world_boss_player_info__init(&my_info);
		my_info.bossid = boss_id;
		my_info.score = my_score;
		my_info.ranknum = my_rank;

	}while(0);

	//answer.result = ret;
	

	fast_send_msg(&connecter, extern_data, MSG_ID_WORLDBOSS_REAL_RANK_INFO_ANSWER, rank_world_boss_real_info_answer__pack, answer);
	return 0;
}
int conn_node_ranksrv::broadcast_world_boss_rank_info(uint64_t boss_id)
{
	char *rank_key = NULL;
	rank_key = get_world_boss_rank_key(boss_id);
	if(rank_key == NULL)
	{
		LOG_ERR("[%s:%d]实时广播世界boss排行榜信息失败,redis key 获取有误,bossid[%lu]", __FUNCTION__, __LINE__, boss_id);
		return -1;
	}

	int ret = 0;
	CRedisClient &rc = sg_redis_client;
	std::vector<std::pair<uint64_t, uint32_t> > cur_world_boss_rank_info;

	RankWorldBossNotify noty;
	rank_world_boss_notify__init(&noty);

	RankWorldBossInfo rank_info[MAX_WORLD_BOSS_REALTINE_RANK_NUM];
	RankWorldBossInfo *rank_info_point[MAX_WORLD_BOSS_REALTINE_RANK_NUM];
	int endNO = MAX_WORLD_BOSS_REALTINE_RANK_NUM -1;
	char name[MAX_WORLD_BOSS_REALTINE_RANK_NUM][MAX_PLAYER_NAME_LEN];

	noty.bossid = boss_id;
	noty.infos = rank_info_point;
	noty.n_infos = 0;
	ret = rc.zget(rank_key, 0, endNO, cur_world_boss_rank_info);
	do
	{
		if(ret != 0 || cur_world_boss_rank_info.size() <= 0)
		{
			break;
		}

		AutoReleaseBatchRedisPlayer t1;	
		std::map<uint64_t, PlayerRedisInfo*> redis_players;
		std::set<uint64_t> notice_player;//所有玩家唯一id
		ret = rc.hkeys(sg_player_key,  notice_player);
		if(ret != 0)
		{
			break;
		}
		if(get_more_redis_player(notice_player ,redis_players, sg_player_key, sg_redis_client, t1) != 0)
		{
			break;
		}

		std::vector<uint64_t> broadcast_ids;//在线玩家
		for (std::set<uint64_t>::iterator iter = notice_player.begin(); iter != notice_player.end(); ++iter)
		{

			PlayerRedisInfo *redis_player = find_redis_from_map(redis_players, *iter);
			if (redis_player && redis_player->status == 0)
			{
				broadcast_ids.push_back(*iter);
			}
		}
		/*for (std::vector<uint64_t>::iterator pp = broadcast_ids.begin(); pp != broadcast_ids.end(); ++pp)
		{
			LOG_ERR("当前在线玩家id[%lu]", *pp);
		}*/
		for(uint32_t i = 0; i <MAX_WORLD_BOSS_REALTINE_RANK_NUM && i < cur_world_boss_rank_info.size(); i++)
		{
			rank_info_point[noty.n_infos] = &rank_info[noty.n_infos];
			rank_world_boss_info__init(&rank_info[noty.n_infos]);
			rank_info[noty.n_infos].player_id = cur_world_boss_rank_info[i].first;
			rank_info[noty.n_infos].score = cur_world_boss_rank_info[i].second;
			PlayerRedisInfo *redis_player = find_redis_from_map(redis_players,rank_info[noty.n_infos].player_id);
			memset(name[noty.n_infos], 0, MAX_PLAYER_NAME_LEN);
			if(redis_player)
			{
				strcpy(name[noty.n_infos], redis_player->name);
				rank_info[noty.n_infos].name = name[noty.n_infos];
			}
			rank_info[noty.n_infos].ranknum = i + 1;

			noty.n_infos++;
		}
		if( broadcast_ids.size() > 0)
		{
			broadcast_message(MSG_ID_WORLDBOSS_CUR_RANK_INFO_NOTIFY, &noty, (pack_func)rank_world_boss_notify__pack,broadcast_ids);
		}
	}while(0);

	/*LOG_ERR("广播的信息-----------------------------");
	for(size_t j =0; j < noty.n_infos; j++)
	{
		LOG_ERR("bossid[%lu] 玩家id[%lu] 玩家名字[%s] 玩家名次[%u] 玩家积分[%u]", noty.bossid, noty.infos[j]->player_id, noty.infos[j]->name, noty.infos[j]->ranknum, noty.infos[j]->score);
	}*/

	return 0;
}

int conn_node_ranksrv::broadcast_message(uint16_t msg_id, void *msg_data, pack_func packer, std::vector<uint64_t> &players)
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
	real_head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD) + len);

	uint64_t *ppp = (uint64_t*)((char *)&head->player_id + len);
	head->num_player_id = 0;
	for (std::vector<uint64_t>::iterator iter = players.begin(); iter != players.end(); ++iter)
	{
		ppp[head->num_player_id++] = *iter;
	}
	head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD_CONN_BROADCAST) + len + sizeof(uint64_t) * head->num_player_id);
	if (conn_node_ranksrv::connecter.send_one_msg((PROTO_HEAD *)head, 1) != (int)(ENDION_FUNC_4(head->len)))
	{
		LOG_ERR("[%s:%d] send to conn_srv failed err[%d]", __FUNCTION__, __LINE__, errno);
	}
	return 0;
}

int conn_node_ranksrv::updata_player_cur_world_boss_info(uint64_t boss_id, EXTERN_DATA *extern_data)
{

	char*rank_key = get_world_boss_rank_key(boss_id);
	if(rank_key == NULL)
	{
		LOG_ERR("[%s:%d]updata_player_cur_world_boss_info faild,boss_id[%lu],  player_id[%lu]", __FUNCTION__, __LINE__, boss_id, extern_data->player_id);
		return -1;
	}
	RankWorldBossPlayerInfo noty;
	
	rank_world_boss_player_info__init(&noty);
	//获取之前的排名
	CRedisClient &rc = sg_redis_client;
	uint64_t player_id = extern_data->player_id;
	uint32_t out_rank = 0xffffffff;
	uint32_t my_rank = 0;
	uint32_t my_score = 0;
	int ret = 0;

	ret = rc.zget_rank(rank_key, player_id, out_rank);
	if (ret == 0)
	{
		out_rank++;
		my_rank = out_rank;
	}
	//获取积分
	ret = rc.zget_score(rank_key, player_id, my_score);
	my_rank = my_rank > MAX_WORLD_BOSS_SHANGBANG_NUM ? 0:my_rank;
	
	noty.bossid = boss_id;
	noty.score = my_score;
	noty.ranknum = my_rank;

	//LOG_ERR("我的信息,bossid [%lu], 积分[%u], 排名[%u]", noty.bossid, noty.score, noty.ranknum);

	fast_send_msg(&connecter, extern_data, MSG_ID_WORLDBOSS_CUR_PLAYER_INFO_NOTIFY, rank_world_boss_player_info__pack, noty);
	return 0;
}

 int conn_node_ranksrv::handle_world_boss_zhu_jiemian_info_request(EXTERN_DATA *extern_data)
{
	RankWorldBossAllBossInfoAnswer answer;
	rank_world_boss_all_boss_info_answer__init(&answer);
	RankWorldBossAllBossInfo world_boss_info[MAX_WORLD_BOSS_NUM];
	RankWorldBossAllBossInfo *world_boss_info_point[MAX_WORLD_BOSS_NUM];
	char last_name[MAX_WORLD_BOSS_NUM][MAX_PLAYER_NAME_LEN];//最后一击玩家名字
	char max_score_name[MAX_WORLD_BOSS_NUM][MAX_PLAYER_NAME_LEN];//上轮最高积分玩家名字
	answer.info = world_boss_info_point;
	answer.n_info = 0;

	AutoReleaseRedisPlayer p1;
	AutoReleaseRankRedisInfo p2;
	for(std::set<uint64_t>::iterator itr = world_boss_id.begin(); itr != world_boss_id.end(); itr++)
	{
		uint64_t boss_id = *itr;
		world_boss_info_point[answer.n_info] =  &world_boss_info[answer.n_info];
		rank_world_boss_all_boss_info__init(&world_boss_info[answer.n_info]);
		world_boss_info[answer.n_info].bossid = boss_id;
		BeforWorldBossRedisinfo *befor_info = get_redis_befor_world_boss(boss_id, befor_world_boss_key, sg_redis_client, p2);
		if(befor_info != NULL)
		{
			strcpy(last_name[answer.n_info], befor_info->name);
			strcpy(max_score_name[answer.n_info], befor_info->max_score_name);
			world_boss_info[answer.n_info].last_name = last_name[answer.n_info];
			world_boss_info[answer.n_info].max_score_name = max_score_name[answer.n_info];
			world_boss_info[answer.n_info].last_player_id = befor_info->player_id;
			world_boss_info[answer.n_info].max_score_player_id = befor_info->max_score_player_id;

		}
		PlayerRedisInfo *last_player_redis = get_redis_player(world_boss_info[answer.n_info].last_player_id, sg_player_key, sg_redis_client, p1);
		if(last_player_redis != NULL)
		{
			world_boss_info[answer.n_info].job = last_player_redis->head_icon;
			world_boss_info[answer.n_info].level  = last_player_redis->lv; 
		}
		CurWorldBossRedisinfo *cur_boss_redis = get_redis_cur_world_boss(boss_id, cur_world_boss_key, sg_redis_client, p2);
		if(cur_boss_redis != NULL)
		{
			world_boss_info[answer.n_info].max_hp = cur_boss_redis->max_hp;
			world_boss_info[answer.n_info].cur_hp = cur_boss_redis->cur_hp;
		}
		answer.n_info++;
	
	}

	fast_send_msg(&connecter, extern_data, MSG_ID_WORLDBOSS_ZHUJIEMIAN_INFO_ANSWER, rank_world_boss_all_boss_info_answer__pack, answer);
	return 0;
}

int conn_node_ranksrv::handle_world_boss_last_rank_info_request(EXTERN_DATA *extern_data)
{

	RankWorldBossLastInfoRequest *req = rank_world_boss_last_info_request__unpack(NULL, get_data_len(), get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] player[%lu] unpack failed", __FUNCTION__, __LINE__, extern_data->player_id);
		return -1;
	}
	uint64_t boss_id = req->bossid;
	rank_world_boss_last_info_request__free_unpacked(req, NULL);


	int ret = 0;
	CRedisClient &rc = sg_redis_client;
	char *rank_key = NULL;
	std::vector<std::pair<uint64_t, uint32_t> > last_world_boss_rank_info;
	uint64_t player_id = extern_data->player_id;
	uint32_t out_rank = 0xffffffff;
	uint32_t my_rank = 0;
	uint32_t my_score = 0;


	RankWorldBossLastInfoAnswer answer;
	rank_world_boss_last_info_answer__init(&answer);
	RankWorldBossInfo rank_info[MAX_WORLD_BOSS_LASTROUND_RANK_NUM];
	RankWorldBossInfo *rank_info_point[MAX_WORLD_BOSS_LASTROUND_RANK_NUM];
	RankWorldBossPlayerInfo my_info;
	int endNO = MAX_WORLD_BOSS_LASTROUND_RANK_NUM -1;
	char name[MAX_WORLD_BOSS_LASTROUND_RANK_NUM][MAX_PLAYER_NAME_LEN];

	answer.bossid = boss_id;
	answer.infos = rank_info_point;
	answer.n_infos = 0;
	do
	{
		rank_key = get_world_boss_rank_key(boss_id*10);
		if( rank_key == NULL)
		{
			ret = 1;
			break;
		}

		ret = rc.zget(rank_key, 0, endNO, last_world_boss_rank_info);
		AutoReleaseBatchRedisPlayer t1;	
		std::map<uint64_t, PlayerRedisInfo*> redis_players;
		std::set<uint64_t> notice_player;//榜单内玩家唯一id
		for(uint32_t i = 0;  i < last_world_boss_rank_info.size(); i++)
		{
			notice_player.insert(last_world_boss_rank_info[i].first);
		}
		
	
		get_more_redis_player(notice_player ,redis_players, sg_player_key, sg_redis_client, t1);
		if(ret == 0 && last_world_boss_rank_info.size() >0)
		{		
			for(uint32_t i = 0; i < MAX_WORLD_BOSS_LASTROUND_RANK_NUM && i < last_world_boss_rank_info.size(); i++)
			{
				rank_info_point[answer.n_infos] = &rank_info[answer.n_infos];
				rank_world_boss_info__init(&rank_info[answer.n_infos]);
				rank_info[answer.n_infos].player_id = last_world_boss_rank_info[i].first;
				rank_info[answer.n_infos].score = last_world_boss_rank_info[i].second;
				PlayerRedisInfo *redis_player = find_redis_from_map(redis_players,rank_info[answer.n_infos].player_id);
				memset(name[answer.n_infos], 0, MAX_PLAYER_NAME_LEN);
				if(redis_player)
				{
					strcpy(name[answer.n_infos], redis_player->name);
					rank_info[answer.n_infos].name = name[answer.n_infos];
				}
				rank_info[answer.n_infos].ranknum = i + 1;

				answer.n_infos++;
			}
		}

		//获取玩家自己的积分和排名信息
		out_rank = 0xffffffff;
		ret = rc.zget_rank(rank_key, player_id, out_rank);
		if (ret == 0)
		{
			out_rank++;
			my_rank = out_rank;
		}
		ret = rc.zget_score(rank_key, player_id, my_score);	

		answer.my_rankinfo = &my_info;
		rank_world_boss_player_info__init(&my_info);
		my_info.bossid = boss_id;
		my_info.score = my_score;
		my_info.ranknum = my_rank;

	}while(0);

	fast_send_msg(&connecter, extern_data, MSG_ID_WORLDBOSS_LAST_RANK_INFO_ANSWER, rank_world_boss_last_info_answer__pack, answer);
	return 0;
}

//世界boss刷新时时间到了，将当前轮信息更新到上轮，在跟新当前轮信息
int conn_node_ranksrv::handle_world_timing_birth_updata_info(EXTERN_DATA *extern_data)
{

	RankWorldBossHpInfo *req = rank_world_boss_hp_info__unpack(NULL, get_data_len(), get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d]  unpack failed", __FUNCTION__, __LINE__);
		return -1;
	}
	
	uint64_t boss_id = req->bossid;
	uint32_t max_hp = req->cur_hp;
	uint32_t cur_hp = req->cur_hp;
	rank_world_boss_hp_info__free_unpacked(req, NULL);
	//LOG_ERR("世界boss出生信息,bossid[%lu], max_hp[%u], cur_hp[%u]", req->bossid, req->max_hp, req->cur_hp);

	char *rank_key = NULL;
	char *befor_rank_key = NULL;
	befor_rank_key = get_world_boss_rank_key(boss_id*10);
	rank_key = get_world_boss_rank_key(boss_id);
	if(rank_key == NULL || befor_rank_key == NULL)
	{	
		LOG_ERR("[%s:%d] 世界boss刷新时更新数据有错", __FUNCTION__, __LINE__);		
		return -2;
	}
	CRedisClient &rc = sg_redis_client;
	char field[128];
	int ret;
	//世界boss不是被打死的，而是时间到了重新刷的，才更新数据
	AutoReleaseRankRedisInfo p1;
	CurWorldBossRedisinfo *cur_boss_redis = get_redis_cur_world_boss(boss_id, cur_world_boss_key, sg_redis_client, p1);
	if(cur_boss_redis != NULL)
	{
		BeforWorldBossRedisinfo befor_boss_dedis;
		befor_world_boss_redisinfo__init(&befor_boss_dedis);
		char last_name[MAX_PLAYER_NAME_LEN]; //上轮最后一击玩家名字
		char max_score_name[MAX_PLAYER_NAME_LEN]; //上轮最高积分玩家名字
		uint64_t max_score_player_id = 0;
		memset(last_name, 0, MAX_PLAYER_NAME_LEN);
		memset(max_score_name, 0, MAX_PLAYER_NAME_LEN);
		befor_boss_dedis.boss_id = boss_id;
		befor_boss_dedis.player_id = cur_boss_redis->player_id;
		strcpy(last_name, cur_boss_redis->name);
		//获取最高积分玩家的数据
		std::vector<std::pair<uint64_t, uint32_t> > befor_world_boss_rank_info;
		rc.zget(rank_key, 0, 0, befor_world_boss_rank_info);
		if(befor_world_boss_rank_info.size() == 1)
		{
			max_score_player_id = befor_world_boss_rank_info[0].first;
		}

		AutoReleaseRedisPlayer p2;
		PlayerRedisInfo *max_score_player_redis = get_redis_player(max_score_player_id, sg_player_key, sg_redis_client, p2);
		if(max_score_player_redis)
		{
			strcpy(max_score_name, max_score_player_redis->name);
			befor_boss_dedis.max_score_name = max_score_name;
			befor_boss_dedis.max_score_player_id = max_score_player_id;
		}
		static uint8_t data_buffer[128 * 1024];
		sprintf(field, "%lu", boss_id);
		do
		{
			size_t data_len = befor_world_boss_redisinfo__pack(&befor_boss_dedis, data_buffer);
			if (data_len == (size_t)-1)
			{
				LOG_ERR("[%s:%d] pack redis world_boss_info failed, bossid[%lu]", __FUNCTION__, __LINE__, boss_id);
				break;
			}

			 ret = rc.hset_bin(befor_world_boss_key, field, (const char *)data_buffer, (int)data_len);
			if (ret < 0)
			{
				LOG_ERR("[%s:%d] set befor world boss failed, bossid[%lu] ret = %d", __FUNCTION__, __LINE__, boss_id, ret);
				break;
			}
		} while(0);
		//将其在当前redis里面的数据清除
		ret = rc.hdel(cur_world_boss_key, field);
		if(ret < 0)
		{
			LOG_ERR("[%s:%d] del cur  world boss failed, bossid[%lu] ret = %d", __FUNCTION__, __LINE__, boss_id, ret)
		}
	}
	//再重置下当前轮信息，主要是设置当前血量信息
	CurWorldBossRedisinfo cur_world_boss_info;
	cur_world_boss_redisinfo__init(&cur_world_boss_info);
	cur_world_boss_info.boss_id = boss_id;
	cur_world_boss_info.max_hp = max_hp;
	cur_world_boss_info.cur_hp = cur_hp;
	static uint8_t data_buffer[128 * 1024];
	sprintf(field, "%lu", boss_id);
	do
	{
		size_t data_len = cur_world_boss_redisinfo__pack(&cur_world_boss_info, data_buffer);
		if (data_len == (size_t)-1)
		{
			LOG_ERR("[%s:%d] pack redis world_boss_info failed, bossid[%lu]", __FUNCTION__, __LINE__, boss_id);
			break;
		}

		ret = rc.hset_bin(cur_world_boss_key, field, (const char *)data_buffer, (int)data_len);
		if (ret < 0)
		{
			LOG_ERR("[%s:%d] set befor world boss failed, bossid[%lu] ret = %d", __FUNCTION__, __LINE__, boss_id, ret);
			break;
		}
	} while(0);

	//将当前排行榜信息跟新到上轮
	do{
		ret = rc.zdel_rank(befor_rank_key, 0, -1);
		if(ret != 0)
		{
			LOG_ERR("[%s:%d]将本轮排行信息跟新到上轮时，清除上轮榜单信息失败[%lu]", __FUNCTION__, __LINE__, boss_id);
			break;
		}
		std::vector<std::pair<uint64_t, uint32_t> > cur_world_boss_rank_info;
		rc.zget(rank_key, 0, -1, cur_world_boss_rank_info);
		for(size_t i =0; i < cur_world_boss_rank_info.size(); ++i)
		{
			ret = rc.zset(befor_rank_key, cur_world_boss_rank_info[i].first, cur_world_boss_rank_info[i].second);
		}
		ret = rc.zdel_rank(rank_key, 0, -1);
		if(ret != 0)
		{
			LOG_ERR("[%s:%d]将本轮排行信息跟新到上轮时，清除本轮榜单信息失败[%lu]", __FUNCTION__, __LINE__, boss_id)	
			break;
		}
	}while(0);

	//发奖励
	if(cur_boss_redis != NULL)
	{
		world_boss_provide_rank_reward(boss_id);
	}

	return 0;
}

//发排行奖励，奖励的发放是在将本轮数据更新到上轮后发放，所以用上轮数据发
int conn_node_ranksrv::world_boss_provide_rank_reward(uint64_t boss_id)
{
	//先发放世界boss击杀奖励
	CRedisClient &rc = sg_redis_client;
	AutoReleaseRankRedisInfo p1;
	int ret;
	char *rank_key = NULL;
	char field[64];
	char world_boss_key[64] = {0};
	uint32_t parame_id =0;
	rank_key = get_world_boss_rank_key(boss_id*10);
	if(rank_key == NULL )
	{	
		LOG_ERR("[%s:%d] 世界boss发放奖励是出错,错误的key:%s", __FUNCTION__, __LINE__, rank_key);		
		return -2;
	}
	WorldBossTable *world_boss_config = get_config_by_id(boss_id, &rank_world_boss_config);
	if(world_boss_config == NULL)
	{
		 LOG_ERR("[%s:%d] 世界boss发放奖励世界boss配置表出错", __FUNCTION__, __LINE__);
		 return -4;
	}

	if(world_boss_config->Type == 1)
	{
		memcpy(world_boss_key, tou_mu_world_boss_reward_num, 64);
		parame_id = tou_mu_parame_id;
	}
	if(world_boss_config->Type == 2)
	{
		memcpy(world_boss_key, shou_ling_world_boss_reward_num, 64);
		parame_id = shou_ling_parame_id;
	}
	uint32_t can_receive_num =0; //可领取奖励最大次数
	ParameterTable *parame_reward_config = get_config_by_id(parame_id, &parameter_config);
	if(parame_reward_config == NULL)
	{
		LOG_ERR("[%s:%d] 世界boss发放排行奖励失败,获取可领取最大奖励次数失败,参数id:%u", __FUNCTION__, __LINE__, parame_id);
		return -5;
	}
	if(parame_reward_config ->n_parameter1 > 0)
	{
		can_receive_num = parame_reward_config ->parameter1[0];
	}


	uint64_t now_time = rank_srv_get_now_time(); //获取当前时间
	//发放排行榜奖励
	std::vector<std::pair<uint64_t, uint32_t> > world_boss_rank_info;
	rc.zget(rank_key, 0, -1, world_boss_rank_info);
	for(size_t i =0; i < world_boss_rank_info.size(); ++i)
	{
		uint64_t player_id = world_boss_rank_info[i].first;
		WorldBossReceiveRewardInfo* reward_info = get_redis_world_boss_receive_reward_info(player_id, world_boss_key, sg_redis_client, p1);
		uint32_t new_num = 0; //新的可领取次数
		if(reward_info != NULL)
		{
			uint64_t last_time = reward_info->last_time;	
			uint32_t reward_num = reward_info->num;
			int flag = judge_the_time_span(last_time, now_time, -1, -1, -1, 0, 0, 0);
			//判断时间有误，报错，不给当前玩家奖励
			if(flag < 0)
			{
				LOG_ERR("[%s:%d] 发放玩家排行奖励失败,player_id[%lu] last_time[%lu] now_time[%lu]", __FUNCTION__, __LINE__, last_time, now_time);
				continue;
			}
			else if(flag == 0)
			{
				//没跨天，不能领了就滤过，可领就将次数加以
				if(reward_num >= can_receive_num)
				{
					continue;
				}
				else
				{
					new_num = reward_num + 1;
				}
			}
			else
			{				
				//跨天了,重新设置奖励次数为1
				new_num = 1;
			}
		}

		//重置可领取奖励次数,以及领奖时间到redis
		if(new_num == 0 )
		{
			new_num = 1;
		}
		WorldBossReceiveRewardInfo new_reward_info;
		world_boss_receive_reward_info__init(&new_reward_info);
		new_reward_info.last_time = now_time;
		new_reward_info.num = new_num;
		if(reward_info != NULL)
		{
			new_reward_info.kill_time = reward_info->kill_time;
			new_reward_info.kill_num = reward_info->kill_num;
		}
		static uint8_t data_buffer_reward[128 * 1024];
		sprintf(field, "%lu", player_id);
		do
		{
			size_t data_len = world_boss_receive_reward_info__pack(&new_reward_info, data_buffer_reward);
			if (data_len == (size_t)-1)
			{
				LOG_ERR("[%s:%d] pack redis world_boss_info failed, bossid[%lu]", __FUNCTION__, __LINE__, boss_id);
				break;
			}

			ret = rc.hset_bin(world_boss_key, field, (const char *)data_buffer_reward, (int)data_len);
			if (ret < 0)
			{
				LOG_ERR("[%s:%d] set befor world boss failed, bossid[%lu] ret = %d", __FUNCTION__, __LINE__, boss_id, ret);
				break;
			}
		} while(0);
		uint32_t rank = i +1;
		receive_world_boss_reward_to_player(rank, boss_id, player_id);
		//最后发奖励
	}
	
	return 0;
}

int conn_node_ranksrv::receive_world_boss_reward_to_player(uint32_t rank, uint64_t boss_id, uint64_t player_id)
{
	uint32_t id = 0;
	if( rank == 1)
	{
		id = WORLD_BOSS_FIRST_GEAR_REWARD;
	}
	else if( rank == 2)
	{
		id = WORLD_BOSS_SECOND_GEAR_REWARD;
	}
	else if(rank == 3)
	{
		id = WORLD_BOSS_THIRD_GEAR_REWARD;
	}
	else if(rank >= 4 && rank <= 10)
	{
		id = WORLD_BOSS_FOURTH_GEAR_REWARD;
	}
	else if(rank >= 11 && rank <= 30)
	{
		id = WORLD_BOSS_FIFTH_GEAR_REWARD;
	}
	else if(rank >= 31 && rank <= 50)
	{
		id = WORLD_BOSS_SIXTH_GEAR_REWARD;
	}
	else if(rank >= 51 && rank <= 70)
	{
		id = WORLD_BOSS_SEVENTH_GEAR_REWARD;
	}
	else if(rank >= 71 && rank <= 100)
	{
		id = WORLD_BOSS_EIGHTH_GEAR_REWARD;
	}
	else
	{
		id = WORLD_BOSS_NINTH_GEAR_REWARD;
	}

	WorldBossRewardTable *reward_config = get_config_by_id(id, &world_boss_reward_config);
	if(reward_config == NULL)
	{
		LOG_ERR("[%s:%d] 世界boss发奖失败,配置错误,player_id[%lu]", __FUNCTION__, __LINE__, player_id);
		return -1;
	}
	std::map<uint32_t, uint32_t> attachs;
	//固定奖励
	if( reward_config->n_ItemID > 0 && reward_config->n_Num >0 && reward_config->n_ItemID == reward_config->n_Num)
	{
		for(size_t i = 0; i < reward_config->n_ItemID; i++)
		{
			attachs[reward_config->ItemID[i]] = reward_config->Num[i];
		}
	}
	//从随机库抽取奖励
	if(reward_config->Draw >0)
	{
		uint32_t luck_num = reward_config->Draw;
		if(reward_config->n_Random >0)
		{
			for(uint32_t i =0; i < luck_num; i ++)
			{
				uint32_t xia_biao = rand() % reward_config->n_Random;
				if(attachs.find(reward_config->Random[xia_biao]) != attachs.end())
				{
					attachs[reward_config->Random[xia_biao]] += 1;
				}
				else
				{
					attachs[reward_config->Random[xia_biao]] = 1;
				}
			}
		}
	}
	//邮件发奖
	send_mail(&connecter, player_id,reward_config->MailID , NULL, NULL, NULL, NULL, &attachs, MAGIC_TYPE_WORLDBOS_RANK_REWARD);

	//通知玩家弹奖励界面
	RankWorldBossRewardNotify notify;
	RankWorldBossRewardInfo item_info[MAX_WORLD_BOSS_REWARD_ITEM_NUM];
	RankWorldBossRewardInfo *item_info_point[MAX_WORLD_BOSS_REWARD_ITEM_NUM];
	rank_world_boss_reward_notify__init(&notify);
	notify.rank = rank;
	notify.bossid = boss_id;
	
	if(attachs.size() >0)
	{
		notify.n_info = 0;
		notify.info = item_info_point;
		for(std::map<uint32_t, uint32_t>::iterator itr = attachs.begin(); itr !=  attachs.end(); itr++)
		{
			item_info_point[notify.n_info] = &item_info[notify.n_info];
			rank_world_boss_reward_info__init(&item_info[notify.n_info]);
			item_info[notify.n_info].item_id = itr->first;
			item_info[notify.n_info].num = itr->second;
			notify.n_info++;
		}
	}
	
	EXTERN_DATA extern_data;
	extern_data.player_id = player_id;
	fast_send_msg(&connecter, &extern_data, MSG_ID_WORLDBOSS_PLAYER_RANK_INFO_NOTIFY, rank_world_boss_reward_notify__pack, notify);
	return 0;
}

//发放击杀奖励
int conn_node_ranksrv::world_boss_provide_kill_reward(uint64_t boss_id)
{
	CRedisClient &rc = sg_redis_client;
	AutoReleaseRankRedisInfo p1;
	AutoReleaseRedisPlayer p2;
	uint64_t player_id =0;
	int ret;
	char *rank_key = NULL;
	char world_boss_key[64] = {0};
	uint32_t parame_id =0;
	uint64_t now_time = rank_srv_get_now_time(); //获取当前时间
	rank_key = get_world_boss_rank_key(boss_id*10);
	if(rank_key == NULL )
	{	
		LOG_ERR("[%s:%d] 世界boss发放奖励是出错,错误的key:%s", __FUNCTION__, __LINE__, rank_key);		
		return -1;
	}
	WorldBossTable *world_boss_config = get_config_by_id(boss_id, &rank_world_boss_config);
	if(world_boss_config == NULL)
	{
		 LOG_ERR("[%s:%d] 世界boss发放奖励世界boss配置表出错", __FUNCTION__, __LINE__);
		 return -2;
	}
	if(world_boss_config->Type == 1)
	{
		memcpy(world_boss_key, tou_mu_world_boss_reward_num, 64);
		parame_id = tou_mu_parame_id;
	}
	if(world_boss_config->Type == 2)
	{
		memcpy(world_boss_key, shou_ling_world_boss_reward_num, 64);
		parame_id = shou_ling_parame_id;
	}
	uint32_t can_receive_num =0; //可领取奖励最大次数
	ParameterTable *parame_reward_config = get_config_by_id(parame_id, &parameter_config);
	if(parame_reward_config == NULL)
	{
		LOG_ERR("[%s:%d] 世界boss发击杀奖励失败,获取可领取最大奖励次数失败,参数id:%u", __FUNCTION__, __LINE__, parame_id);
		return -3;
	}
	if(parame_reward_config ->n_parameter1 > 0)
	{
		can_receive_num = parame_reward_config ->parameter1[0];
	}

	BeforWorldBossRedisinfo *info = get_redis_befor_world_boss(boss_id, befor_world_boss_key, sg_redis_client, p1);
	if(info == NULL)
	{
		LOG_ERR("[%s:%d] get redis befor world boss info faild boosid=%lu", __FUNCTION__, __LINE__, boss_id);
		return -5;
	}

	player_id = info->player_id;
	if(player_id == 0)
	{
		return -6;
	}
	PlayerRedisInfo * last_player_info = get_redis_player( player_id, sg_player_key, sg_redis_client, p2);
	if(last_player_info == NULL)
	{
		LOG_ERR("[%s:%d] 世界boss发最后一刀奖励获取玩家redis信息失败player_id=%lu", __FUNCTION__, __LINE__, player_id);
		return -7;;
	}
	if(last_player_info->lv > world_boss_config->RewardLevel)
	{
		LOG_DEBUG("[%s:%d] 世界boss发最后一刀奖励失败,玩家等级超过等级限制player_id=%lu player_lv=%u max_lv=%lu", __FUNCTION__, __LINE__, player_id, last_player_info->lv, world_boss_config->RewardLevel);
		return -8;
	}
	WorldBossReceiveRewardInfo* reward_info = get_redis_world_boss_receive_reward_info(player_id, world_boss_key, sg_redis_client, p1);
	uint32_t new_num = 0; //新的可领取次数
	if(reward_info != NULL)
	{
		uint64_t last_time = reward_info->kill_time;	
		uint32_t reward_num = reward_info->kill_num;
		int flag = judge_the_time_span(last_time, now_time, -1, -1, -1, 0, 0, 0);
		//判断时间有误，报错，不给当前玩家奖励
		if(flag < 0)
		{
			LOG_ERR("[%s:%d] 发放最后一刀奖励失败,player_id[%lu] last_time[%lu] now_time[%lu]", __FUNCTION__, __LINE__, last_time, now_time);
			return -9;
		}
		else if(flag == 0)
		{
			//没跨天，不能领了就滤过，可领就将次数加以
			if(reward_num >= can_receive_num)
			{
				return -10;
			}
			else
			{
				new_num = reward_num + 1;
			}
		}
		else
		{				
			//跨天了,重新设置奖励次数为1
			new_num = 1;
		}
	}

	//重置可领取奖励次数,以及领奖时间到redis
	if(new_num == 0 )
	{
		new_num = 1;
	}
	WorldBossReceiveRewardInfo new_reward_info;
	world_boss_receive_reward_info__init(&new_reward_info);
	if(reward_info != NULL)
	{
		new_reward_info.last_time = reward_info->last_time;
		new_reward_info.num = reward_info->num;
	}
	new_reward_info.kill_time = now_time;
	new_reward_info.kill_num = new_num;
	static uint8_t data_buffer_reward[128 * 1024];
	char field[64];
	sprintf(field, "%lu", player_id);
	do
	{
		size_t data_len = world_boss_receive_reward_info__pack(&new_reward_info, data_buffer_reward);
		if (data_len == (size_t)-1)
		{
			LOG_ERR("[%s:%d] pack  world_boss_receive_reward_info failed, bossid[%lu]", __FUNCTION__, __LINE__, boss_id);
			return -11;
		}

		ret = rc.hset_bin(world_boss_key, field, (const char *)data_buffer_reward, (int)data_len);
		if (ret < 0)
		{
			LOG_ERR("[%s:%d] set bworld_boss_receive_reward_info failed, bossid[%lu] ret = %d", __FUNCTION__, __LINE__, boss_id, ret);
			return -12;
		}
	} while(0);

	ParameterTable *parame_config = get_config_by_id(161000326, &parameter_config);
	std::map<uint32_t, uint32_t> attachs;
	if( parame_config != NULL && parame_config->n_parameter1 != 0 && parame_config->n_parameter1%2 == 0)
	{
		for(size_t i =0; i < parame_config->n_parameter1; i=i+2)
		{
			attachs[parame_config->parameter1[i]] = parame_config->parameter1[i+1];
		}
		send_mail(&connecter, player_id,MAIL_ID_WORLDBOSS_KILL_REWAERD , NULL, NULL, NULL, NULL, &attachs, MAGIC_TYPE_WORLDBOS_KILL_REWARD);
	}
		
	return 0;
}
