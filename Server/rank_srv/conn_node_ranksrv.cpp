#include "conn_node_ranksrv.h"
#include "game_event.h"
#include "msgid.h"
#include "error_code.h"
#include "time_helper.h"
#include <vector>
#include <map>
#include <set>
#include "redis_client.h"
#include "rank.pb-c.h"
#include "player_redis_info.pb-c.h"


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
	init_rank_key_map();
	char rank_key[128];
	for (std::map<uint32_t, std::string>::iterator iter = scm_rank_keys.begin(); iter != scm_rank_keys.end(); ++iter)
	{
		sprintf(rank_key, iter->second.c_str(), server_id);
		iter->second.assign(rank_key);
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

class AutoReleaseBatchRedisPlayer
{
public:
	AutoReleaseBatchRedisPlayer() {}
	~AutoReleaseBatchRedisPlayer()
	{
		for (std::vector<PlayerRedisInfo*>::iterator iter = pointer_vec.begin(); iter != pointer_vec.end(); ++iter)
		{
			player_redis_info__free_unpacked(*iter, NULL);
		}
	}

	void push_back(PlayerRedisInfo *player)
	{
		pointer_vec.push_back(player);
	}
private:
	std::vector<PlayerRedisInfo *> pointer_vec;
};

conn_node_ranksrv::conn_node_ranksrv()
{
	max_buf_len = 1024 * 1024;
	buf = (uint8_t *)malloc(max_buf_len + sizeof(EXTERN_DATA));
	assert(buf);
	
	add_msg_handle(SERVER_PROTO_REFRESH_PLAYER_REDIS_INFO, &conn_node_ranksrv::handle_refresh_player_info);
	add_msg_handle(MSG_ID_RANK_INFO_REQUEST, &conn_node_ranksrv::handle_rank_info_request);
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

static PlayerRedisInfo *get_redis_player(uint64_t player_id)
{
	CRedisClient &rc = sg_redis_client;
	static uint8_t data_buffer[32 * 1024];
	int data_len = 32 * 1024;
	char field[64];
	sprintf(field, "%lu", player_id);
	int ret = rc.hget_bin(sg_player_key, field, (char *)data_buffer, &data_len);
	if (ret == 0)
	{
		return player_redis_info__unpack(NULL, data_len, data_buffer);
	}

	return NULL;
}

int get_more_redis_player(std::set<uint64_t> &player_ids, std::map<uint64_t, PlayerRedisInfo*> &redis_players)
{
	if (player_ids.size() == 0)
	{
		return 0;
	}

	std::vector<std::relation_three<uint64_t, char*, int> > player_infos;
	for (std::set<uint64_t>::iterator iter = player_ids.begin(); iter != player_ids.end(); ++iter)
	{
		std::relation_three<uint64_t, char*, int> tmp(*iter, NULL, 0);
		player_infos.push_back(tmp);
	}

	int ret = sg_redis_client.get(sg_player_key, player_infos);
	if (ret != 0)
	{
		LOG_ERR("[%s:%d] hmget failed, ret:%d", __FUNCTION__, __LINE__, ret);
		return -1;
	}

	for (std::vector<std::relation_three<uint64_t, char*, int> >::iterator iter = player_infos.begin(); iter != player_infos.end(); ++iter)
	{
		PlayerRedisInfo *redis_player = player_redis_info__unpack(NULL, iter->three, (uint8_t*)iter->second);
		if (!redis_player)
		{
			LOG_ERR("[%s:%d] unpack redis failed, player_id:%lu", __FUNCTION__, __LINE__, iter->first);
			return -1;
		}

		redis_players[iter->first] = redis_player;
	}

	return 0;
}

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

int conn_node_ranksrv::handle_refresh_player_info(EXTERN_DATA *extern_data)
{
	int proto_data_len = get_data_len();
	uint8_t *proto_data = get_data();
	uint32_t refresh_type = *((uint32_t*)proto_data);
	PlayerRedisInfo *req = player_redis_info__unpack(NULL, proto_data_len - sizeof(uint32_t), proto_data + sizeof(uint32_t));
	if (!req)
	{
		LOG_ERR("[%s:%d] player[%lu] unpack req failed, refresh_type:%u", __FUNCTION__, __LINE__, extern_data->player_id, refresh_type);
		return -1;
	}

	LOG_INFO("[%s:%d] player[%lu], refresh_type:%u, status:%u", __FUNCTION__, __LINE__, extern_data->player_id, refresh_type, req->status);

	CRedisClient &rc = sg_redis_client;
	char field[128];
	sprintf(field, "%lu", extern_data->player_id);
	char *rank_key = NULL;
	int ret = 0;
	uint64_t player_id = extern_data->player_id;

	AutoReleaseBatchRedisPlayer arb_redis;
	arb_redis.push_back(req);
	PlayerRedisInfo *redis_player = get_redis_player(extern_data->player_id);
	if (redis_player)
	{
		arb_redis.push_back(redis_player);
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
			rank_key = get_rank_key(RANK_LEVEL_TOTAL);
			ret = rc.zset(rank_key, player_id, level);
			if (ret != 0)
			{
				LOG_ERR("[%s:%d] update %s %lu failed, score:%u, old_score:%u", __FUNCTION__, __LINE__, rank_key, player_id, level, old_level);
			}

			rank_key = get_rank_key(RANK_LEVEL_TOTAL + job);
			ret = rc.zset(rank_key, player_id, level);
			if (ret != 0)
			{
				LOG_ERR("[%s:%d] update %s %lu failed, score:%u, old_score:%u", __FUNCTION__, __LINE__, rank_key, player_id, level, old_level);
			}
		}

		if (fc_total != old_fc_total)
		{
			rank_key = get_rank_key(RANK_FC_TOTAL);
			ret = rc.zset(rank_key, player_id, fc_total);
			if (ret != 0)
			{
				LOG_ERR("[%s:%d] update %s %lu failed, score:%u, old_score:%u", __FUNCTION__, __LINE__, rank_key, player_id, fc_total, old_fc_total);
			}

			rank_key = get_rank_key(RANK_FC_TOTAL + job);
			ret = rc.zset(rank_key, player_id, fc_total);
			if (ret != 0)
			{
				LOG_ERR("[%s:%d] update %s %lu failed, score:%u, old_score:%u", __FUNCTION__, __LINE__, rank_key, player_id, fc_total, old_fc_total);
			}
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
			rank_key = get_rank_key(RANK_EQUIP_TOTAL);
			ret = rc.zset(rank_key, player_id, fc_equip);
			if (ret != 0)
			{
				LOG_ERR("[%s:%d] update %s %lu failed, score:%u, old_score:%u", __FUNCTION__, __LINE__, rank_key, player_id, fc_equip, old_fc_equip);
			}

			rank_key = get_rank_key(RANK_EQUIP_TOTAL + job);
			ret = rc.zset(rank_key, player_id, fc_equip);
			if (ret != 0)
			{
				LOG_ERR("[%s:%d] update %s %lu failed, score:%u, old_score:%u", __FUNCTION__, __LINE__, rank_key, player_id, fc_equip, old_fc_equip);
			}
		}

		if (fc_bagua != old_fc_bagua)
		{
			rank_key = get_rank_key(RANK_BAGUA_TOTAL);
			ret = rc.zset(rank_key, player_id, fc_bagua);
			if (ret != 0)
			{
				LOG_ERR("[%s:%d] update %s %lu failed, score:%u, old_score:%u", __FUNCTION__, __LINE__, rank_key, player_id, fc_bagua, old_fc_bagua);
			}

			rank_key = get_rank_key(RANK_BAGUA_TOTAL + job);
			ret = rc.zset(rank_key, player_id, fc_bagua);
			if (ret != 0)
			{
				LOG_ERR("[%s:%d] update %s %lu failed, score:%u, old_score:%u", __FUNCTION__, __LINE__, rank_key, player_id, fc_bagua, old_fc_bagua);
			}
		}

		if (coin != old_coin)
		{
			rank_key = get_rank_key(RANK_TREASURE_COIN);
			ret = rc.zset(rank_key, player_id, coin);
			if (ret != 0)
			{
				LOG_ERR("[%s:%d] update %s %lu failed, score:%u, old_score:%u", __FUNCTION__, __LINE__, rank_key, player_id, coin, old_coin);
			}
		}

		if (gold != old_gold)
		{
			rank_key = get_rank_key(RANK_TREASURE_GOLD);
			ret = rc.zset(rank_key, player_id, gold);
			if (ret != 0)
			{
				LOG_ERR("[%s:%d] update %s %lu failed, score:%u, old_score:%u", __FUNCTION__, __LINE__, rank_key, player_id, gold, old_gold);
			}
		}

		if (gold_bind != old_gold_bind)
		{
			rank_key = get_rank_key(RANK_TREASURE_BIND_GOLD);
			ret = rc.zset(rank_key, player_id, gold_bind);
			if (ret != 0)
			{
				LOG_ERR("[%s:%d] update %s %lu failed, score:%u, old_score:%u", __FUNCTION__, __LINE__, rank_key, player_id, gold_bind, old_gold_bind);
			}
		}

		if (pvp3_score != old_pvp3_score)
		{
			if (pvp3_division > 1)
			{
				rank_key = get_rank_key(RANK_PVP3_DIVISION2 + pvp3_division - 2);
				ret = rc.zset(rank_key, player_id, pvp3_score);
				if (ret != 0)
				{
					LOG_ERR("[%s:%d] update %s %lu failed, score:%u, old_score:%u", __FUNCTION__, __LINE__, rank_key, player_id, pvp3_score, old_pvp3_score);
				}
			}

			if (pvp3_division != old_pvp3_division)
			{
				rank_key = get_rank_key(RANK_PVP3_DIVISION2 + old_pvp3_division - 2);
				std::vector<uint64_t> dels;
				dels.push_back(player_id);
				ret = rc.zdel(rank_key, dels);
				if (ret != 0)
				{
					LOG_ERR("[%s:%d] del %s %lu failed, score:%u, old_score:%u", __FUNCTION__, __LINE__, rank_key, player_id, pvp3_score, old_pvp3_score);
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

		if (get_more_redis_player(playerIds, redis_players) != 0)
		{
			ret = ERROR_ID_RANK_REDIS;
			LOG_ERR("[%s:%d] player[%lu] get player failed, rank_type:%lu", __FUNCTION__, __LINE__, extern_data->player_id, rank_type);
			break;
		}

		if (my_rank == 0)
		{
			sg_redis_client.zget_rank(rank_key, extern_data->player_id, my_rank);
			if (my_rank >= MAX_RANK_ADD_NUM)
			{
				my_rank = 0;
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

	for (std::map<uint64_t, PlayerRedisInfo*>::iterator iter = redis_players.begin(); iter != redis_players.end(); ++iter)
	{
		player_redis_info__free_unpacked(iter->second, NULL);
	}
	return 0;
}


