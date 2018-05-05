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
#include "zhenying.pb-c.h"
#include "send_mail.h"
#include "app_data_statis.h"
#include "rank_config.h"
#include "rank_util.h" 

conn_node_ranksrv conn_node_ranksrv::connecter;

CRedisClient sg_redis_client;
uint32_t sg_server_id;
struct event sg_clear_timer_event;
struct timeval sg_clear_timer_val = {3600, 0};	
struct event sg_reward_timer_event;
struct timeval sg_reward_timer_val;	

char sg_player_key[64]; //玩家数据
char sg_guild_key[64]; //门宗数据
char sg_reward_key[64]; //发奖时间
static std::map<uint32_t, std::string> scm_rank_keys;
static std::map<uint32_t, char *> rank_key_map;

static int on_guild_player_fc_change(uint32_t guild_id);
void cb_reward_timeout(evutil_socket_t, short, void* /*arg*/);


#define MAX_RANK_GET_NUM  100 //前端显示数目
#define MAX_RANK_ADD_NUM  5000 //最多排行数
#define MAX_RANK_ATTR_NUM  12
//斗法场最大排名
#define DOUFACHANG_MAX_RANK 7000

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
	//阵营军阶
	RANK_ZHENYING_LEVEL_TOTAL = 701,
	RANK_ZHENYING_LEVEL_ZHENYING1 = 718,
	RANK_ZHENYING_LEVEL_ZHENYING2 = 719,
	//阵营功勋
	RANK_ZHENYING_EXPLOIT_TOATAL = 801,
	RANK_ZHENYING_EXPLOIT_ZHENYING1 = 818,
	RANK_ZHENYING_EXPLOIT_ZHENYING2 = 819,
	//阵营斩杀
	RANK_ZHENYING_KILL_TOTAL = 901,
	RANK_ZHENYING_KILL_ZHENYING1 = 918,
	RANK_ZHENYING_KILL_ZHENYING2 = 919,
	//门宗战力
	RANK_GUILD_FC_TOTAL = 1001,
	RANK_GUILD_FC_ZHENYING1 = 1018,
	RANK_GUILD_FC_ZHENYING2 = 1019,
	//门宗功勋
	RANK_GUILD_EXPLOIT_TOTAL = 1101,
	RANK_GUILD_EXPLOIT_ZHENYING1 = 1118,
	RANK_GUILD_EXPLOIT_ZHENYING2 = 1119,
	//门宗人气
	RANK_GUILD_POPULAR_TOTAL = 1201,
	RANK_GUILD_POPULAR_ZHENYING1 = 1218,
	RANK_GUILD_POPULAR_ZHENYING2 = 1219,
	//有奖答题
	RANK_AWARD_QUESTION_TOTAL = 1301,
	//斗法场排行(1400 - 1499 斗法场专用)
	RANK_DOU_FA_CHANG_TOTAL = 1401,
	RANK_DOU_FA_CHANG_ZHENYING1 = 1418,
	RANK_DOU_FA_CHANG_ZHENYING2 = 1419,

	//魅力值
	RANK_MEI_LI_ZHI_TATAL = 1501,
	RANK_MEI_LI_ZHI_ZHENYING1 = 1518,
	RANK_MEI_LI_ZHI_ZHENYING2 = 1519,
	//最强伙伴
	RANK_ZUI_QIANG_PARTNER_TATAL = 1601,
	RANK_ZUI_QIANG_PARTNER_ZHENYING1 = 1618,
	RANK_ZUI_QIANG_PARTNER_ZHENYING2 = 1619,

	//幻宝地牢
	RANK_HUAN_BAO_DI_NAO_MAX_LAYER_TATAL = 1701,
	RANK_HUAN_BAO_DI_NAO_MAX_LAYER_ZHENYING1 = 1718,
	RANK_HUAN_BAO_DI_NAO_MAX_LAYER_ZHENYING2 = 1719,
	
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
	scm_rank_keys[RANK_ZHENYING_LEVEL_TOTAL]                   = "s%u_rank_zhenying_level_total";
	scm_rank_keys[RANK_ZHENYING_LEVEL_ZHENYING1]               = "s%u_rank_zhenying_level_zhenying1";
	scm_rank_keys[RANK_ZHENYING_LEVEL_ZHENYING2]               = "s%u_rank_zhenying_level_zhenying2";
	scm_rank_keys[RANK_ZHENYING_EXPLOIT_TOATAL]                = "s%u_rank_zhenying_exploit_total";
	scm_rank_keys[RANK_ZHENYING_EXPLOIT_ZHENYING1]             = "s%u_rank_zhenying_exploit_zhenying1";
	scm_rank_keys[RANK_ZHENYING_EXPLOIT_ZHENYING2]             = "s%u_rank_zhenying_exploit_zhenying2";
	scm_rank_keys[RANK_ZHENYING_KILL_TOTAL]                    = "s%u_rank_zhenying_kill_total";
	scm_rank_keys[RANK_ZHENYING_KILL_ZHENYING1]                = "s%u_rank_zhenying_kill_zhenying1";
	scm_rank_keys[RANK_ZHENYING_KILL_ZHENYING2]                = "s%u_rank_zhenying_kill_zhenying2";
	scm_rank_keys[RANK_GUILD_FC_TOTAL]                         = "s%u_rank_guild_fc_total";
	scm_rank_keys[RANK_GUILD_FC_ZHENYING1]                     = "s%u_rank_guild_fc_zhenying1";
	scm_rank_keys[RANK_GUILD_FC_ZHENYING2]                     = "s%u_rank_guild_fc_zhenying2";
	scm_rank_keys[RANK_GUILD_EXPLOIT_TOTAL]                    = "s%u_rank_guild_exploit_total";
	scm_rank_keys[RANK_GUILD_EXPLOIT_ZHENYING1]                = "s%u_rank_guild_exploit_zhenying1";
	scm_rank_keys[RANK_GUILD_EXPLOIT_ZHENYING2]                = "s%u_rank_guild_exploit_zhenying2";
	scm_rank_keys[RANK_GUILD_POPULAR_TOTAL]                    = "s%u_rank_guild_popular_total";
	scm_rank_keys[RANK_GUILD_POPULAR_ZHENYING1]                = "s%u_rank_guild_popular_zhenying1";
	scm_rank_keys[RANK_GUILD_POPULAR_ZHENYING2]                = "s%u_rank_guild_popular_zhenying2";
	scm_rank_keys[RANK_AWARD_QUESTION_TOTAL]                   = "s%u_rank_award_question";
	scm_rank_keys[RANK_DOU_FA_CHANG_TOTAL]					   = "doufachang_rank_%u";
	scm_rank_keys[RANK_DOU_FA_CHANG_ZHENYING1]				   = "doufachang_rank_%u";
	scm_rank_keys[RANK_DOU_FA_CHANG_ZHENYING2]				   = "doufachang_rank_%u";
	scm_rank_keys[RANK_MEI_LI_ZHI_TATAL]					   = "s%u_rank_mei_li_zhi_total";
	scm_rank_keys[RANK_MEI_LI_ZHI_ZHENYING1]				   = "s%u_rank_mei_li_zhi_zhengying1";
	scm_rank_keys[RANK_MEI_LI_ZHI_ZHENYING2]				   = "s%u_rank_mei_li_zhi_zhengying2";
	scm_rank_keys[RANK_ZUI_QIANG_PARTNER_TATAL]				   = "s%u_rank_zui_qiang_partner_tatal";
	scm_rank_keys[RANK_ZUI_QIANG_PARTNER_ZHENYING1]			   = "s%u_rank_zui_qiang_partner_zhengying1";
	scm_rank_keys[RANK_ZUI_QIANG_PARTNER_ZHENYING2]			   = "s%u_rank_zui_qiang_partner_zhengying2";
	scm_rank_keys[RANK_HUAN_BAO_DI_NAO_MAX_LAYER_TATAL]		   = "s%u_rank_huan_bao_di_nao_max_yayer_tatal";
	scm_rank_keys[RANK_HUAN_BAO_DI_NAO_MAX_LAYER_ZHENYING1]	   = "s%u_rank_huan_bao_di_nao_max_yayer_zhengying1";
	scm_rank_keys[RANK_HUAN_BAO_DI_NAO_MAX_LAYER_ZHENYING2]	   = "s%u_rank_huan_bao_di_nao_max_yayer_zhengying2";
}

void init_redis_keys(uint32_t server_id)
{
	sprintf(sg_player_key, "server_%u", server_id);
	sprintf(sg_guild_key, "s%u_guild", server_id);
	sprintf(sg_reward_key, "s%u_rank_reward_time", server_id);
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

class AutoReleaseBatchRedisGuild
{
public:
	AutoReleaseBatchRedisGuild() {}
	~AutoReleaseBatchRedisGuild()
	{
		for (std::vector<RedisGuildInfo*>::iterator iter = pointer_vec.begin(); iter != pointer_vec.end(); ++iter)
		{
			redis_guild_info__free_unpacked(*iter, NULL);
		}
	}

	void push_back(RedisGuildInfo *guild)
	{
		pointer_vec.push_back(guild);
	}
private:
	std::vector<RedisGuildInfo *> pointer_vec;
};

RedisGuildInfo *get_redis_guild(uint32_t guild_id, char *guild_key, CRedisClient &rc, AutoReleaseBatchRedisGuild &_pool)
{
	static uint8_t data_buffer[32 * 1024];
	int data_len = sizeof(data_buffer);
	char field[64];
	sprintf(field, "%u", guild_id);
	int ret = rc.hget_bin(guild_key, field, (char *)data_buffer, &data_len);
	if (ret == 0)
	{
		RedisGuildInfo *ret = redis_guild_info__unpack(NULL, data_len, data_buffer);
		if (ret)
			_pool.push_back(ret);
		return ret;
	}

	return NULL;
}

int get_more_redis_guild(std::set<uint32_t> &guild_ids, std::map<uint32_t, RedisGuildInfo*> &redis_guilds,
	char *guild_key, CRedisClient &rc, AutoReleaseBatchRedisGuild &_pool)
{
	if (guild_ids.size() == 0)
	{
		return 0;
	}

	std::vector<std::relation_three<uint64_t, char*, int> > guild_infos;
	for (std::set<uint32_t>::iterator iter = guild_ids.begin(); iter != guild_ids.end(); ++iter)
	{
		std::relation_three<uint64_t, char*, int> tmp(*iter, NULL, 0);
		guild_infos.push_back(tmp);
	}

	int ret = rc.get(guild_key, guild_infos);
	if (ret != 0)
	{
		LOG_ERR("[%s:%d] hmget failed, ret:%d", __FUNCTION__, __LINE__, ret);
		return -1;
	}

	for (std::vector<std::relation_three<uint64_t, char*, int> >::iterator iter = guild_infos.begin(); iter != guild_infos.end(); ++iter)
	{
		RedisGuildInfo *redis_guild = redis_guild_info__unpack(NULL, iter->three, (uint8_t*)iter->second);
		if (!redis_guild)
		{
			ret = -1;
			LOG_ERR("[%s:%d] unpack redis failed, guild_id:%lu", __FUNCTION__, __LINE__, iter->first);
			continue;
		}

		redis_guilds[iter->first] = redis_guild;
		_pool.push_back(redis_guild);
	}

	for (std::vector<std::relation_three<uint64_t, char*, int> >::iterator iter = guild_infos.begin(); iter != guild_infos.end(); ++iter)
	{
		free(iter->second);
	}

	return ret;
}

conn_node_ranksrv::conn_node_ranksrv()
{
	max_buf_len = 1024 * 1024;
	buf = (uint8_t *)malloc(max_buf_len + sizeof(EXTERN_DATA));
	assert(buf);
	
	add_msg_handle(SERVER_PROTO_REFRESH_PLAYER_REDIS_INFO, &conn_node_ranksrv::handle_refresh_player_info);
	add_msg_handle(MSG_ID_RANK_INFO_REQUEST, &conn_node_ranksrv::handle_rank_info_request);
	add_msg_handle(SERVER_PROTO_PLAYER_ONLINE_NOTIFY, &conn_node_ranksrv::handle_player_online_notify);
	add_msg_handle(SERVER_PROTO_REFRESH_GUILD_REDIS_INFO, &conn_node_ranksrv::handle_refresh_guild_info);
	add_msg_handle(MSG_ID_GET_ZHENYING_LEADER_REQUEST, &conn_node_ranksrv::handle_zhenying_leader);

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
RedisGuildInfo *find_guild_from_map(std::map<uint32_t, RedisGuildInfo*> &redis_guilds, uint32_t guild_id)
{
	std::map<uint32_t, RedisGuildInfo*>::iterator iter = redis_guilds.find(guild_id);
	if (iter != redis_guilds.end())
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

uint32_t get_reward_time_from_redis(void)
{
	std::string sTime = sg_redis_client.get(sg_reward_key);
	if (sTime.length() > 0)
	{
		return strtoul(sTime.c_str(), NULL, 10);
	}
	return 0;
}

void set_reward_time_to_redis(uint32_t ts)
{
	char val[22];
	sprintf(val, "%u", ts);
	sg_redis_client.set(sg_reward_key, val);
}

time_t calcu_reward_timer(void)
{
	uint32_t now = time_helper::get_cached_time() / 1000;
	time_t t = get_reward_time_from_redis();
	uint32_t hour = sg_rank_reward_time / 100, min = sg_rank_reward_time % 100;
	uint32_t interval = sg_rank_reward_interval * 24 * 3600;
	if (t == 0)
	{
		t = time_helper::get_day_timestamp(hour, min, 0, now) + (sg_rank_reward_first_interval - 1) * 24 * 3600;
	}
	else if (now >= t)
	{
		if (now - t > interval)
		{
			t = time_helper::get_day_timestamp(hour, min, 0, now) + interval;
		}
		else
		{
			t += interval;
		}
	}

	return t;
}

void set_reward_timer()
{
	uint32_t now = time_helper::get_cached_time() / 1000;
	time_t next_ts = calcu_reward_timer();
	set_reward_time_to_redis(next_ts);

	sg_reward_timer_val.tv_sec = next_ts - now;
	sg_reward_timer_event.ev_callback = cb_reward_timeout;
	add_timer(sg_reward_timer_val, &sg_reward_timer_event, NULL);
}

void do_reward_job()
{
	LOG_INFO("[%s:%d] send rank reward mail", __FUNCTION__, __LINE__);
	//发放奖励
	uint32_t reward_rank = 0;
	uint32_t rank_type = 0;
	char*    rank_name = NULL;
	char*    rank_key = NULL;
	std::vector<std::pair<uint64_t, uint32_t> > rank_info;
	std::map<uint32_t, uint32_t> attachs;
	std::vector<char *> mail_args;
	std::stringstream ss;
	char sz_rank[12];
	for (std::map<uint64_t, std::vector<struct RankingRewardTable *> >::iterator iter = rank_reward_map.begin(); iter != rank_reward_map.end(); ++iter)
	{
		reward_rank = 0;
		rank_info.clear();
		rank_type = iter->first;
		rank_name = NULL;
		rank_key  = NULL;
		for (std::vector<RankingRewardTable*>::iterator iter_vec = iter->second.begin(); iter_vec != iter->second.end(); ++iter_vec)
		{
			RankingRewardTable *config = *iter_vec;
			reward_rank = std::max(reward_rank, (uint32_t)config->RankEnd);
			if (rank_name == NULL)
			{
				for (uint32_t i = 0; i < config->n_RankingTableId; ++i)
				{
					if (config->RankingTableId[i] == (uint64_t)rank_type)
					{
						rank_name = config->RankingTableIdName[i];
						break;
					}
				}
			}
		}

		rank_key = get_rank_key(rank_type);
		if (!rank_key)
		{
			continue;
		}

		int ret2 = sg_redis_client.zget(rank_key, 0, reward_rank - 1, rank_info);
		if (ret2 != 0)
		{
			continue;
		}

		mail_args.clear();
		mail_args.push_back(rank_name);
		mail_args.push_back(sz_rank);
		for (std::vector<RankingRewardTable*>::iterator iter_vec = iter->second.begin(); iter_vec != iter->second.end(); ++iter_vec)
		{
			RankingRewardTable *config = *iter_vec;
			attachs.clear();
			for (uint32_t i = 0; i < config->n_Reward; ++i)
			{
				attachs[config->Reward[i]] += config->RewardNum[i];
			}

			for (uint64_t rank = config->RankStart; rank <= config->RankEnd; ++rank)
			{
				if (rank > rank_info.size())
				{
					break;
				}

				ss.str("");
				ss.clear();
				ss << rank;
				ss >> sz_rank;
				
				send_mail(&conn_node_ranksrv::connecter, rank_info[rank - 1].first, 270100010, NULL, NULL, NULL, &mail_args, &attachs, MAGIC_TYPE_RANKING_REWARD);
			}
		}
	}
	
	set_reward_timer();
}

void init_reward_timer(void)
{
	uint64_t now = time_helper::get_micro_time();
	time_helper::set_cached_time(now / 1000);

	uint32_t reward_ts = get_reward_time_from_redis();
	if (reward_ts != 0 && reward_ts <= now / 1000000)
	{
		do_reward_job();
	}
	else
	{
		set_reward_timer();
	}
}

void cb_reward_timeout(evutil_socket_t, short, void* /*arg*/)
{
	uint64_t now = time_helper::get_micro_time();
	time_helper::set_cached_time(now / 1000);

	do_reward_job();
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
//	if (prev_rank == post_rank)
//	{
//		return ;
//	}

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
		pPlayer[proto->num].score = out_vec[i].second;
		proto->num++;
	}

	proto->head.len = ENDION_FUNC_4(sizeof(PROTO_SYNC_RANK_CHANGE) + sizeof(ProtoRankPlayer) * proto->num);

	if (conn_node_ranksrv::connecter.send_one_msg(&proto->head, 1) != (int)ENDION_FUNC_4(proto->head.len))
	{
		LOG_ERR("[%s:%d] send to game_srv failed err[%d]", __FUNCTION__, __LINE__, errno);
		return ;
	}
}

//更新玩家在排行榜中的分数
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

//将玩家从排行榜中删除
static void del_player_rank(uint32_t rank_type, uint64_t player_id)
{
	char *rank_key = NULL;
	uint32_t post_rank = 0xffffffff;
	uint32_t prev_rank = 0xffffffff;
	CRedisClient &rc = sg_redis_client;
	rank_key = get_rank_key(rank_type);
	prev_rank = get_player_rank(rank_type, player_id);
	std::vector<uint64_t> dels;
	dels.push_back(player_id);
	int ret = rc.zdel(rank_key, dels);
	if (ret != 0)
	{
		LOG_ERR("[%s:%d] del %s %lu failed", __FUNCTION__, __LINE__, rank_key, player_id);
	}
	else
	{
//		post_rank = get_player_rank(rank_type, player_id);
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

	LOG_INFO("[%s:%d] player[%lu], refresh_type:%u, status:%u, zhenying:%u, fc:%u",
		__FUNCTION__, __LINE__, extern_data->player_id, refresh_type, req->status, req->zhenying, req->fighting_capacity);

	CRedisClient &rc = sg_redis_client;
	char field[128];
	sprintf(field, "%lu", extern_data->player_id);
	uint32_t rank_type = 0;
//	char *rank_key = NULL;
//	int ret = 0;
	uint64_t player_id = extern_data->player_id;
//	uint32_t post_rank = 0xffffffff;
//	uint32_t prev_rank = 0xffffffff;

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
		uint32_t zhenying = req->zhenying;
		uint32_t zhenying_level = req->zhenying_level;
		uint32_t zhenying_kill = req->zhenying_kill;
		uint32_t exploit = req->exploit;
		uint32_t award_question = req->award_question;
		MaxPowerPartner *new_max_partner = req->maxpartner;
		uint32_t new_meili = req->meili_num;
		uint32_t new_max_tower = req->max_tower;
		
		uint32_t old_level = 0;
		uint32_t old_fc_total = 0;
		uint32_t old_fc_equip = 0;
		uint32_t old_fc_bagua = 0;
		uint32_t old_coin = 0;
		uint32_t old_gold = 0;
		uint32_t old_gold_bind = 0;
		uint32_t old_pvp3_division = 0;
		uint32_t old_pvp3_score = 0;
		uint32_t old_zhenying = 0;
		uint32_t old_zhenying_level = 0;
		uint32_t old_zhenying_kill = 0;
		uint32_t old_exploit = 0;
		uint32_t old_question = 0;
		MaxPowerPartner *old_max_partner = NULL;
		uint32_t old_meili = 0;
		uint32_t old_max_tower = 0;

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
			old_zhenying = redis_player->zhenying;
			old_zhenying_level = redis_player->zhenying_level;
			old_zhenying_kill = redis_player->zhenying_kill;
			old_exploit = redis_player->exploit;
			old_question = redis_player->award_question;
			old_max_partner = redis_player->maxpartner;
			old_meili = redis_player->meili_num;
			old_max_tower = redis_player->max_tower;
		}
		else
		{
			save_player = req;
		}

		if(old_max_tower != new_max_tower)
		{
			rank_type = RANK_HUAN_BAO_DI_NAO_MAX_LAYER_TATAL;
			update_player_rank_score(rank_type, player_id, 0, new_max_tower);
			if (zhenying > 0)
			{
				rank_type = RANK_HUAN_BAO_DI_NAO_MAX_LAYER_ZHENYING1 + zhenying - 1;
				update_player_rank_score(rank_type, player_id, 0, new_max_tower);
			}
		
		}
		if(old_meili != new_meili)
		{
			rank_type = RANK_MEI_LI_ZHI_TATAL;
			update_player_rank_score(rank_type, player_id, 0, new_meili);
			if (zhenying > 0)
			{
				rank_type = RANK_MEI_LI_ZHI_ZHENYING1 + zhenying - 1;
				update_player_rank_score(rank_type, player_id, 0, new_meili);
			}
		
		}

		if(old_max_partner != NULL)
		{
			if(new_max_partner != NULL)
			{
				if(old_max_partner->id != new_max_partner->id || old_max_partner->power != new_max_partner->power)
				{
					rank_type = RANK_ZUI_QIANG_PARTNER_TATAL;
					update_player_rank_score(rank_type, player_id, 0, new_max_partner->power);
					if (zhenying > 0)
					{
						rank_type = RANK_ZUI_QIANG_PARTNER_ZHENYING1 + zhenying - 1;
						update_player_rank_score(rank_type, player_id, 0, new_max_partner->power);
					}
				}

			}
			else 
			{
				rank_type = RANK_ZUI_QIANG_PARTNER_TATAL;
				del_player_rank(rank_type, player_id);
				if(zhenying > 0)
				{
					rank_type = RANK_ZUI_QIANG_PARTNER_ZHENYING1 + zhenying - 1;
					del_player_rank(rank_type, player_id);
				}
			}
		}
		else 
		{
			if(new_max_partner != NULL)
			{
				rank_type = RANK_ZUI_QIANG_PARTNER_TATAL;
				update_player_rank_score(rank_type, player_id, 0, new_max_partner->power);
				if (zhenying > 0)
				{
					rank_type = RANK_ZUI_QIANG_PARTNER_ZHENYING1 + zhenying - 1;
					update_player_rank_score(rank_type, player_id, 0, new_max_partner->power);
				}
			
			}
		}
		if (zhenying != old_zhenying)
		{
			if(old_zhenying == 0)
			{
				if (new_max_partner != NULL)
				{
					rank_type = RANK_ZUI_QIANG_PARTNER_ZHENYING1 + zhenying - 1;
					update_player_rank_score(rank_type, player_id, 0, new_max_partner->power);
				}

				rank_type =  RANK_MEI_LI_ZHI_ZHENYING1 + zhenying - 1;
				update_player_rank_score(rank_type, player_id, 0, new_meili);

				rank_type = RANK_HUAN_BAO_DI_NAO_MAX_LAYER_ZHENYING1 + zhenying - 1;
				update_player_rank_score(rank_type, player_id, 0, new_max_tower);
			}
			else 
			{
				if(zhenying == 0)
				{
					rank_type = RANK_ZUI_QIANG_PARTNER_ZHENYING1 + old_zhenying - 1;
					del_player_rank(rank_type, player_id);

					rank_type = RANK_MEI_LI_ZHI_ZHENYING1 + old_zhenying - 1;
					del_player_rank(rank_type, player_id);

					rank_type = RANK_HUAN_BAO_DI_NAO_MAX_LAYER_ZHENYING1 + old_zhenying - 1;
					del_player_rank(rank_type, player_id);
				}
				else 
				{
					if (new_max_partner != NULL)
					{
						rank_type = RANK_ZUI_QIANG_PARTNER_ZHENYING1 + zhenying - 1;
						update_player_rank_score(rank_type, player_id, 0, new_max_partner->power);
					}

					rank_type = RANK_ZUI_QIANG_PARTNER_ZHENYING1 + old_zhenying - 1;
					del_player_rank(rank_type, player_id);

					rank_type = RANK_MEI_LI_ZHI_ZHENYING1 + zhenying - 1;
					update_player_rank_score(rank_type, player_id, 0, new_meili);
					rank_type = RANK_MEI_LI_ZHI_ZHENYING1 + old_zhenying - 1;
					del_player_rank(rank_type, player_id);

					rank_type = RANK_HUAN_BAO_DI_NAO_MAX_LAYER_ZHENYING1 + zhenying - 1;
					update_player_rank_score(rank_type, player_id, 0, new_max_tower);
					rank_type = RANK_HUAN_BAO_DI_NAO_MAX_LAYER_ZHENYING1 + old_zhenying - 1;
					del_player_rank(rank_type, player_id);
				}
			
			}
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
			if (req->guild_id > 0)
			{
				on_guild_player_fc_change(req->guild_id);
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

			if (pvp3_division != old_pvp3_division && old_pvp3_division > 1)
			{
				rank_type = RANK_PVP3_DIVISION2 + old_pvp3_division - 2;
				del_player_rank(rank_type, player_id);
			}
		}

		if (zhenying_level != old_zhenying_level || zhenying != old_zhenying)
		{
			if (zhenying > 0)
			{
				rank_type = RANK_ZHENYING_LEVEL_TOTAL;
				update_player_rank_score(rank_type, player_id, old_zhenying_level, zhenying_level);

				rank_type = RANK_ZHENYING_LEVEL_ZHENYING1 + zhenying - 1;
				update_player_rank_score(rank_type, player_id, old_zhenying_level, zhenying_level);
			}

			if (zhenying != old_zhenying && old_zhenying > 0)
			{
				rank_type = RANK_ZHENYING_LEVEL_ZHENYING1 + old_zhenying - 1;
				del_player_rank(rank_type, player_id);
			}
		}

		if (exploit != old_exploit || zhenying != old_zhenying)
		{
			if (zhenying > 0)
			{
				rank_type = RANK_ZHENYING_EXPLOIT_TOATAL;
				update_player_rank_score(rank_type, player_id, old_exploit, exploit);

				rank_type = RANK_ZHENYING_EXPLOIT_ZHENYING1 + zhenying - 1;
				update_player_rank_score(rank_type, player_id, old_exploit, exploit);
			}

			if (zhenying != old_zhenying && old_zhenying > 0)
			{
				rank_type = RANK_ZHENYING_EXPLOIT_ZHENYING1 + old_zhenying - 1;
				del_player_rank(rank_type, player_id);
			}

			if (exploit != old_exploit && req->guild_id > 0)
			{
				on_guild_player_fc_change(req->guild_id);
			}
		}

		if (zhenying_kill != old_zhenying_kill || zhenying != old_zhenying)
		{
			if (zhenying > 0)
			{
				rank_type = RANK_ZHENYING_KILL_TOTAL;
				update_player_rank_score(rank_type, player_id, old_zhenying_kill, zhenying_kill);

				rank_type = RANK_ZHENYING_KILL_ZHENYING1 + zhenying - 1;
				update_player_rank_score(rank_type, player_id, old_zhenying_kill, zhenying_kill);
			}

			if (zhenying != old_zhenying && old_zhenying > 0)
			{
				rank_type = RANK_ZHENYING_KILL_ZHENYING1 + old_zhenying - 1;
				del_player_rank(rank_type, player_id);
			}
		}

		if (award_question != old_question)
		{
			rank_type = RANK_AWARD_QUESTION_TOTAL;
			update_player_rank_score(rank_type, player_id, old_question, award_question);
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
	rank->baseinfo->attrs[rank->baseinfo->n_attrs]->id = PLAYER_ATTR_ZHENYING;
	rank->baseinfo->attrs[rank->baseinfo->n_attrs]->val = redis->zhenying;
	rank->baseinfo->n_attrs++;
	rank->baseinfo->attrs[rank->baseinfo->n_attrs]->id = PLAYER_ATTR_SEX;
	rank->baseinfo->attrs[rank->baseinfo->n_attrs]->val = redis->sex;
	rank->baseinfo->n_attrs++;
	rank->baseinfo->attrs[rank->baseinfo->n_attrs]->id = PLAYER_ATTR_FIGHTING_CAPACITY;
	rank->baseinfo->attrs[rank->baseinfo->n_attrs]->val = redis->fighting_capacity;
	rank->baseinfo->n_attrs++;

	rank->baseinfo->tags = redis->tags;
	rank->baseinfo->n_tags = redis->n_tags;
	rank->baseinfo->textintro = redis->textintro;

	rank->guildid = redis->guild_id;
	rank->guildname = redis->guild_name;
}

static int handle_guild_rank_info(EXTERN_DATA *extern_data, uint32_t rank_type)
{
	int ret = 0;
	std::vector<std::pair<uint64_t, uint32_t> > rank_info;
	AutoReleaseBatchRedisPlayer t1;		
	std::map<uint64_t, PlayerRedisInfo *> redis_players;
	AutoReleaseBatchRedisGuild t2;		
	std::map<uint32_t, RedisGuildInfo *> redis_guilds;
	uint32_t my_rank = 0;
	do
	{
		char *rank_key = get_rank_key(rank_type);
		if (rank_key == NULL)
		{
			ret = ERROR_ID_RANK_TYPE;
			LOG_ERR("[%s:%d] player[%lu] rank type, rank_type:%u", __FUNCTION__, __LINE__, extern_data->player_id, rank_type);
			break;
		}

		int ret2 = sg_redis_client.zget(rank_key, 0, 99, rank_info);
		if (ret2 != 0)
		{
			ret = ERROR_ID_RANK_REDIS;
			LOG_ERR("[%s:%d] player[%lu] get rank failed, rank_type:%u, rank_key:%s", __FUNCTION__, __LINE__, extern_data->player_id, rank_type, rank_key);
			break;
		}

		std::set<uint32_t> guildIds;
		for (size_t i = 0; i < rank_info.size(); ++i)
		{
			guildIds.insert(rank_info[i].first);
		}

		if (get_more_redis_guild(guildIds, redis_guilds, sg_guild_key, sg_redis_client, t2) != 0)
		{
			ret = ERROR_ID_RANK_REDIS;
			LOG_ERR("[%s:%d] player[%lu] get guild failed, rank_type:%u", __FUNCTION__, __LINE__, extern_data->player_id, rank_type);
			break;
		}

		std::set<uint64_t> playerIds;
		for (std::map<uint32_t, RedisGuildInfo *>::iterator iter = redis_guilds.begin(); iter != redis_guilds.end(); ++iter)
		{
			playerIds.insert(iter->second->master_id);
		}

		if (get_more_redis_player(playerIds, redis_players, sg_player_key, sg_redis_client, t1) != 0)
		{
			ret = ERROR_ID_RANK_REDIS;
			LOG_ERR("[%s:%d] player[%lu] get player failed, rank_type:%u", __FUNCTION__, __LINE__, extern_data->player_id, rank_type);
			break;
		}
	} while(0);

	RankInfoAnswer resp;
	rank_info_answer__init(&resp);

	RankGuildData  rank_data[MAX_RANK_GET_NUM];
	RankGuildData* rank_point[MAX_RANK_GET_NUM];

	resp.result = ret;
	resp.type = rank_type;
	resp.myrank = my_rank;
	resp.guildranks = rank_point;
	resp.n_guildranks = 0;
	for (size_t i = 0; i < rank_info.size(); ++i)
	{
		uint32_t guild_id = rank_info[i].first;
		uint32_t score = rank_info[i].second;

		rank_point[resp.n_guildranks] = &rank_data[resp.n_guildranks];
		rank_guild_data__init(&rank_data[resp.n_guildranks]);
		rank_data[resp.n_guildranks].guildid = guild_id;

		RedisGuildInfo *redis_guild = find_guild_from_map(redis_guilds, guild_id);
		if (redis_guild)
		{
			rank_data[resp.n_guildranks].guildname = redis_guild->name;
			rank_data[resp.n_guildranks].zhenying = redis_guild->zhenying;
			rank_data[resp.n_guildranks].masterid = redis_guild->master_id;
			rank_data[resp.n_guildranks].lv = redis_guild->level;
			rank_data[resp.n_guildranks].popular = redis_guild->popularity;
			rank_data[resp.n_guildranks].icon = redis_guild->head;
			
			PlayerRedisInfo *redis_player = find_redis_from_map(redis_players, redis_guild->master_id);
			if (redis_player)
			{
				rank_data[resp.n_guildranks].mastername = redis_player->name;
			}
		}

		rank_data[resp.n_guildranks].ranknum = i + 1;
		rank_data[resp.n_guildranks].score = score;
		resp.n_guildranks++;
	}

	fast_send_msg(&conn_node_ranksrv::connecter, extern_data, MSG_ID_RANK_INFO_ANSWER, rank_info_answer__pack, resp);
	return 0;
}

static int handle_dou_fa_chang_rank_info(EXTERN_DATA *extern_data, uint64_t rank_type)
{
	int ret = 0;
	//std::vector<std::pair<uint64_t, uint32_t> > rank_info;
	AutoReleaseBatchRedisPlayer t1;		
	std::map<uint64_t, PlayerRedisInfo *> redis_players;
	uint64_t my_rank = 0;
	uint32_t my_score = 0;
	uint64_t my_player_id = extern_data->player_id;
	uint64_t rank_player_id[MAX_RANK_GET_NUM] = {0};
	uint64_t rank_player_rank[MAX_RANK_GET_NUM] = {0};
	uint64_t rank_player_id_zhengying1[MAX_RANK_GET_NUM] = {0};
	uint64_t rank_player_id_zhengying2[MAX_RANK_GET_NUM] = {0};
	for(uint64_t i = 0; i < MAX_RANK_GET_NUM; i++)
	{
		rank_player_rank[i] = i + 1;
	}
	do
	{
		char *rank_key = get_rank_key(rank_type);
		if (rank_key == NULL)
		{
			ret = ERROR_ID_RANK_TYPE;
			LOG_ERR("[%s:%d] player[%lu] rank type, rank_type:%lu", __FUNCTION__, __LINE__, extern_data->player_id, rank_type);
			break;
		}
		sg_redis_client.mget_uint64(rank_key, MAX_RANK_GET_NUM, rank_player_rank, rank_player_id);

		std::set<uint64_t> playerIds;
		for (size_t i = 0; i < MAX_RANK_GET_NUM; ++i)
		{
			if(rank_player_id[i] == 0)
				break;
			playerIds.insert(rank_player_id[i]);

			if (rank_player_id[i] == extern_data->player_id)
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
			sg_redis_client.mget_uint64(rank_key, 1, &my_player_id, &my_rank);
		}

		for (size_t i = 0; i < MAX_RANK_GET_NUM; ++i)
		{
			uint64_t player_id = rank_player_id[i];
			if(player_id == 0)
				break;
			PlayerRedisInfo *redis_player = find_redis_from_map(redis_players, player_id);
			if(redis_player == NULL)
				continue;
			for(size_t j = 0; j < MAX_RANK_GET_NUM; j++)
			{
				if(redis_player->zhenying == 1)
				{
					if(rank_player_id_zhengying1[j] == 0)
					{
						rank_player_id_zhengying1[j] = player_id;
						break;
					}
				}
				else 
				{
					if(rank_player_id_zhengying1[j] == 0)
					{
						rank_player_id_zhengying2[j] = player_id;
						break;
					}
					
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
	resp.type = rank_type;
	resp.myrank = my_rank;
	resp.myscore = my_score;
	resp.infos = rank_point;
	resp.n_infos = 0;
	for (size_t i = 0; i < MAX_RANK_GET_NUM; ++i)
	{
		uint64_t player_id = 0;
		switch(rank_type)
		{
			case RANK_DOU_FA_CHANG_TOTAL:
				player_id = rank_player_id[i];
				break;
			case RANK_DOU_FA_CHANG_ZHENYING1:
				player_id = rank_player_id_zhengying1[i];
				break;
			case RANK_DOU_FA_CHANG_ZHENYING2:
				player_id = rank_player_id_zhengying2[i];
				break;
			default:
				break;
		}
		if(player_id == 0)
			break;
		PlayerRedisInfo *redis_player = find_redis_from_map(redis_players, player_id);
		if (redis_player == NULL)
		{
			continue;
		}

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
		
		fill_rank_player(redis_player, &rank_data[resp.n_infos]);

		rank_data[resp.n_infos].baseinfo->playerid = player_id;
		rank_data[resp.n_infos].ranknum = i + 1;
		rank_data[resp.n_infos].score = redis_player->fighting_capacity;
		resp.n_infos++;
	}

	fast_send_msg(&conn_node_ranksrv::connecter, extern_data, MSG_ID_RANK_INFO_ANSWER, rank_info_answer__pack, resp);

	return 0;
}
static int handle_rank_zui_qiang_partner_info_request(EXTERN_DATA *extern_data, uint64_t rank_type)
{

	int ret = 0;
	std::vector<std::pair<uint64_t, uint32_t> > rank_info;
	AutoReleaseBatchRedisPlayer t1;		
	std::map<uint64_t, PlayerRedisInfo *> redis_players;
	uint32_t my_rank = 0, my_score = 0;
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

		sg_redis_client.zget_score(rank_key, extern_data->player_id, my_score);
	} while(0);

	RankInfoAnswer resp;
	rank_info_answer__init(&resp);

	RankPlayerData  rank_data[MAX_RANK_GET_NUM];
	RankPlayerData* rank_point[MAX_RANK_GET_NUM];
	PlayerBaseData  base_data[MAX_RANK_GET_NUM];
	AttrData  attr_data[MAX_RANK_GET_NUM][MAX_RANK_ATTR_NUM];
	AttrData* attr_point[MAX_RANK_GET_NUM][MAX_RANK_ATTR_NUM];

	resp.result = ret;
	resp.type = rank_type;
	resp.myrank = my_rank;
	resp.myscore = my_score;
	resp.infos = rank_point;
	resp.n_infos = 0;
	for (size_t i = 0; i < rank_info.size() && resp.n_infos < MAX_RANK_GET_NUM; ++i)
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
		if (redis_player && redis_player->maxpartner)
		{
			fill_rank_player(redis_player, &rank_data[resp.n_infos]);
			rank_data[resp.n_infos].score = score;
			rank_data[resp.n_infos].baseinfo->playerid = player_id;
			rank_data[resp.n_infos].ranknum = resp.n_infos + 1;
			rank_data[resp.n_infos].partner_id = redis_player->maxpartner->id;
			rank_data[resp.n_infos].partner_level = redis_player->maxpartner->level;
			rank_data[resp.n_infos].partner_name = redis_player->maxpartner->partner_name;
			resp.n_infos++;
		}

	}

	fast_send_msg(&conn_node_ranksrv::connecter, extern_data, MSG_ID_RANK_INFO_ANSWER, rank_info_answer__pack, resp);

	return 0;
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

	if (rank_type / 100 == 10)
	{
		return handle_guild_rank_info(extern_data, rank_type);
	}

	if(rank_type / 100 == 14)
	{
		return handle_dou_fa_chang_rank_info(extern_data, rank_type);
	}

	if(rank_type / 100 == 16)
	{
		return handle_rank_zui_qiang_partner_info_request(extern_data, rank_type);
	}

	int ret = 0;
	std::vector<std::pair<uint64_t, uint32_t> > rank_info;
	AutoReleaseBatchRedisPlayer t1;		
	std::map<uint64_t, PlayerRedisInfo *> redis_players;
	uint32_t my_rank = 0, my_score = 0;
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

		sg_redis_client.zget_score(rank_key, extern_data->player_id, my_score);
	} while(0);

	RankInfoAnswer resp;
	rank_info_answer__init(&resp);

	RankPlayerData  rank_data[MAX_RANK_GET_NUM];
	RankPlayerData* rank_point[MAX_RANK_GET_NUM];
	PlayerBaseData  base_data[MAX_RANK_GET_NUM];
	AttrData  attr_data[MAX_RANK_GET_NUM][MAX_RANK_ATTR_NUM];
	AttrData* attr_point[MAX_RANK_GET_NUM][MAX_RANK_ATTR_NUM];

	resp.result = ret;
	resp.type = rank_type;
	resp.myrank = my_rank;
	resp.myscore = my_score;
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
		rank_data[resp.n_infos].score = score;
		if (redis_player)
		{
			fill_rank_player(redis_player, &rank_data[resp.n_infos]);
			if((rank_type == RANK_ZHENYING_LEVEL_TOTAL || rank_type == RANK_ZHENYING_LEVEL_ZHENYING1 || rank_type == RANK_ZHENYING_LEVEL_ZHENYING2) && redis_player->zhenying != 0)
			{
				rank_data[resp.n_infos].score = 360200000 + redis_player->zhenying * 10000 + score;
			}
		}

		rank_data[resp.n_infos].baseinfo->playerid = player_id;
		rank_data[resp.n_infos].ranknum = i + 1;
		resp.n_infos++;
	}

	fast_send_msg(&connecter, extern_data, MSG_ID_RANK_INFO_ANSWER, rank_info_answer__pack, resp);

	// for (std::map<uint64_t, PlayerRedisInfo*>::iterator iter = redis_players.begin(); iter != redis_players.end(); ++iter)
	// {
	// 	player_redis_info__free_unpacked(iter->second, NULL);
	// }
	return 0;
}
/*
static const int MAX_ZHENYING_GUILD = 5;
static int get_zhenying_guild()
{
	int ret = 0;
	std::vector<std::pair<uint64_t, uint32_t> > rank_info;
	AutoReleaseBatchRedisPlayer t1;
	std::map<uint64_t, PlayerRedisInfo *> redis_players;
	AutoReleaseBatchRedisGuild t2;
	std::map<uint32_t, RedisGuildInfo *> redis_guilds;
	uint32_t my_rank = 0;
	uint32_t rank_type = RANK_GUILD_EXPLOIT_ZHENYING1;
	while (rank_type <= RANK_GUILD_EXPLOIT_ZHENYING2)
	{
		char *rank_key = get_rank_key(rank_type);
		if (rank_key == NULL)
		{
			ret = ERROR_ID_RANK_TYPE;
			LOG_ERR("[%s:%d]  rank type", __FUNCTION__, __LINE__);
			continue;
		}

		int ret2 = sg_redis_client.zget(rank_key, 0, 4, rank_info);
		if (ret2 != 0)
		{
			ret = ERROR_ID_RANK_REDIS;
			LOG_ERR("[%s:%d] get rank failed, rank_key:%s", __FUNCTION__, __LINE__, rank_key);
			continue;
		}

		std::set<uint32_t> guildIds;
		for (size_t i = 0; i < rank_info.size(); ++i)
		{
			guildIds.insert(rank_info[i].first);
		}

		if (get_more_redis_guild(guildIds, redis_guilds, sg_guild_key, sg_redis_client, t2) != 0)
		{
			ret = ERROR_ID_RANK_REDIS;
			LOG_ERR("[%s:%d] get guild failed", __FUNCTION__, __LINE__);
			continue;
		}

		std::set<uint64_t> playerIds;
		for (std::map<uint32_t, RedisGuildInfo *>::iterator iter = redis_guilds.begin(); iter != redis_guilds.end(); ++iter)
		{
			playerIds.insert(iter->second->master_id);
		}

		if (get_more_redis_player(playerIds, redis_players, sg_player_key, sg_redis_client, t1) != 0)
		{
			ret = ERROR_ID_RANK_REDIS;
			LOG_ERR("[%s:%d]  get player failed, rank_type:%lu", __FUNCTION__, __LINE__, rank_type);
			continue;
		}
		++rank_type;
	};

	RankInfoAnswer resp;
	rank_info_answer__init(&resp);

	RankGuildData  rank_data[MAX_RANK_GET_NUM];
	RankGuildData* rank_point[MAX_RANK_GET_NUM];

	resp.result = ret;
	resp.type = rank_type;
	resp.myrank = my_rank;
	resp.guildranks = rank_point;
	resp.n_guildranks = 0;
	for (size_t i = 0; i < rank_info.size(); ++i)
	{
		uint32_t guild_id = rank_info[i].first;
		uint32_t score = rank_info[i].second;

		rank_point[resp.n_guildranks] = &rank_data[resp.n_guildranks];
		rank_guild_data__init(&rank_data[resp.n_guildranks]);
		rank_data[resp.n_guildranks].guildid = guild_id;

		RedisGuildInfo *redis_guild = find_guild_from_map(redis_guilds, guild_id);
		if (redis_guild)
		{
			rank_data[resp.n_guildranks].guildname = redis_guild->name;
			rank_data[resp.n_guildranks].zhenying = redis_guild->zhenying;
			rank_data[resp.n_guildranks].masterid = redis_guild->master_id;
			PlayerRedisInfo *redis_player = find_redis_from_map(redis_players, redis_guild->master_id);
			if (redis_player)
			{
				rank_data[resp.n_guildranks].mastername = redis_player->name;
			}
		}

		rank_data[resp.n_guildranks].ranknum = i + 1;
		rank_data[resp.n_guildranks].score = score;
		resp.n_guildranks++;
	}

	//fast_send_msg(&conn_node_ranksrv::connecter, extern_data, MSG_ID_RANK_INFO_ANSWER, rank_info_answer__pack, resp);
	return 0;
}*/
int conn_node_ranksrv::handle_zhenying_leader(EXTERN_DATA *extern_data)
{
	static const int MAX_LEADER = 3;
	static const int MAX_ZHENYING_GUILD = 5;
	uint32_t rankKey[2][MAX_LEADER] = {
		{ RANK_ZHENYING_LEVEL_ZHENYING1,
		//阵营功勋
		RANK_ZHENYING_EXPLOIT_ZHENYING1,
		//阵营斩杀
		RANK_ZHENYING_KILL_ZHENYING1 
		},

		{ RANK_ZHENYING_LEVEL_ZHENYING2,
		RANK_ZHENYING_EXPLOIT_ZHENYING2,
		RANK_ZHENYING_KILL_ZHENYING2 
		}, 
		};

	ZhenyingNumberOne numOne[2][MAX_LEADER];
	ZhenyingNumberOne *pNumOne[2][MAX_LEADER];
	std::vector<std::pair<uint64_t, uint32_t> > rank_info;
	AutoReleaseBatchRedisPlayer t1;
	std::map<uint64_t, PlayerRedisInfo *> redis_players;
	std::set<uint64_t> playerIds;
	for (uint32_t z = 0; z < 2; ++z)
	{
		for (int i = 0; i < MAX_LEADER; ++i)
		{
			//pNumOne[z][i] = NULL; 
			pNumOne[z][i] = &numOne[z][i];
			zhenying_number_one__init(&numOne[z][i]);
			numOne[z][i].playerid = 0;
			rank_info.clear();
			char *rank_key = get_rank_key(rankKey[z][i]);
			if (rank_key == NULL)
			{
				LOG_ERR("[%s:%d] player[%lu] rank type, rank_type:%u", __FUNCTION__, __LINE__, extern_data->player_id, rankKey[z][i]);
				continue;
			}

			int ret2 = sg_redis_client.zget(rank_key, 0, 0, rank_info);
			if (ret2 != 0)
			{
				LOG_ERR("[%s:%d] player[%lu] get rank failed, rank_type:%u, rank_key:%s", __FUNCTION__, __LINE__, extern_data->player_id, rankKey[z][i], rank_key);
				continue;
			}
			if (rank_info.size() == 0)
			{
				continue;
			}
			playerIds.insert(rank_info[0].first);
			//AutoReleaseRedisPlayer p1;
			//PlayerRedisInfo *rplayer = get_redis_player(rank_info[i].first, sg_player_key, sg_redis_client, p1);
			//if (rplayer == NULL)
			//{
			//	continue;
			//}
			
			numOne[z][i].playerid = rank_info[0].first;
		}
	}
	
	ZhenyingLeader send;
	zhenying_leader__init(&send);
	send.fulongguo = &pNumOne[0][0];
	send.dianfenggu = &pNumOne[1][0];
	send.n_fulongguo = send.n_dianfenggu = MAX_LEADER;
	ZhenyingGuild guild[2][MAX_ZHENYING_GUILD];
	ZhenyingGuild *pGuild[2][MAX_ZHENYING_GUILD];
	send.fulongguo_guild = &pGuild[0][0];
	send.dianfenggu_guild = &pGuild[1][0];
	AutoReleaseBatchRedisGuild t2;
	std::map<uint32_t, RedisGuildInfo *> redis_guilds;
	uint32_t rank_type = RANK_GUILD_EXPLOIT_ZHENYING1;
	uint32_t zhenying = 1;
	for (; rank_type <= RANK_GUILD_EXPLOIT_ZHENYING2; 	++rank_type, ++zhenying)
	{
		rank_info.clear();
		char *rank_key = get_rank_key(rank_type);
		if (rank_key == NULL)
		{
			LOG_ERR("[%s:%d]  rank type", __FUNCTION__, __LINE__);
			continue;
		}

		int ret2 = sg_redis_client.zget(rank_key, 0, 4, rank_info);
		if (ret2 != 0)
		{
			LOG_ERR("[%s:%d] get rank failed, rank_key:%s", __FUNCTION__, __LINE__, rank_key);
			continue;
		}
		//test
		//for (uint32_t t = 4194310; t < 4194313; ++t)
		//{
		//	rank_info.push_back(std::make_pair(t, t));
		//}

		std::set<uint32_t> guildIds;
		for (size_t i = 0; i < rank_info.size(); ++i)
		{
			pGuild[zhenying - 1][i] = &guild[zhenying - 1][i];
			zhenying_guild__init(pGuild[zhenying - 1][i]);
			guild[zhenying - 1][i].guild_id = rank_info[i].first;
			//guild[numGuild].zhengying = zhenying;
			guild[zhenying - 1][i].rank = i + 1;
			guildIds.insert(rank_info[i].first);
		}
		if (zhenying == 1)
		{
			send.n_fulongguo_guild = rank_info.size();
		}
		else
		{
			send.n_dianfenggu_guild = rank_info.size();
		}

		if (get_more_redis_guild(guildIds, redis_guilds, sg_guild_key, sg_redis_client, t2) != 0)
		{
			LOG_ERR("[%s:%d] get guild failed", __FUNCTION__, __LINE__);
			continue;
		}

		for (std::map<uint32_t, RedisGuildInfo *>::iterator iter = redis_guilds.begin(); iter != redis_guilds.end(); ++iter)
		{
			playerIds.insert(iter->second->master_id);
		}
	}

	if (get_more_redis_player(playerIds, redis_players, sg_player_key, sg_redis_client, t1) != 0)
	{
		LOG_ERR("[%s:%d] player[%lu] get player failed,", __FUNCTION__, __LINE__, extern_data->player_id);
	}
	for (uint32_t z = 0; z < 2; ++z)
	{
		for (int i = 0; i < MAX_LEADER; ++i)
		{
			if (pNumOne[z][i] != NULL)
			{
				PlayerRedisInfo *rplayer = find_redis_from_map(redis_players, pNumOne[z][i]->playerid);
				if (rplayer)
				{
					//numOne[z][i].playerid = rank_info[i].first;
					numOne[z][i].name = rplayer->name;
					numOne[z][i].job = rplayer->job;
					numOne[z][i].lv = rplayer->lv;
					numOne[z][i].head = rplayer->head_icon;
					numOne[z][i].kill = rplayer->zhenying_kill;
					numOne[z][i].step = rplayer->zhenying_level;
					numOne[z][i].expoit = rplayer->exploit;
				}
			}
		}
	}

	zhenying = 0;
	for (; zhenying < 2; ++zhenying)
	{
		uint32_t s = zhenying == 0 ? send.n_fulongguo_guild : send.n_dianfenggu_guild;
		for (size_t i = 0; i < s; ++i)
		{
			RedisGuildInfo *redis_guild = find_guild_from_map(redis_guilds, guild[zhenying][i].guild_id);
			if (redis_guild)
			{
				guild[zhenying][i].guild_name = redis_guild->name;
				guild[zhenying][i].zhengying = redis_guild->zhenying;
				guild[zhenying][i].exploit = redis_guild->exploit;
				guild[zhenying][i].guild_head = redis_guild->head;
				//rank_data[resp.n_guildranks].masterid = redis_guild->master_id;
				PlayerRedisInfo *redis_player = find_redis_from_map(redis_players, redis_guild->master_id);
				if (redis_player)
				{
					guild[zhenying][i].leader_name = redis_player->name;
					guild[zhenying][i].leader_head = redis_guild->exploit;
					guild[zhenying][i].exploit = redis_guild->exploit;
				}
				guild[zhenying][i].leader_id = redis_guild->master_id;
			}
		}
	}
	
	//ZhenyingNumberOneSide fulongguo, dianfenggu;
	//zhenying_number_one_side__init(&fulongguo);
	//zhenying_number_one_side__init(&dianfenggu);
	//fulongguo.number_one_step = pNumOne[0];
	//fulongguo.number_one_kill = pNumOne[4];
	//fulongguo.number_one_contribute = pNumOne[2];
	//dianfenggu.number_one_step = pNumOne[1];
	//dianfenggu.number_one_kill = pNumOne[5];
	//dianfenggu.number_one_contribute = pNumOne[3];
	
	fast_send_msg(&connecter, extern_data, MSG_ID_GET_ZHENYING_LEADER_ANSWER, zhenying_leader__pack, send);

	return 0;
}

int conn_node_ranksrv::handle_player_online_notify(EXTERN_DATA *extern_data)
{
	handle_zhenying_leader(extern_data);
	int ret = 0;
	do
	{
		uint32_t out_rank = 0xffffffff;
		uint32_t out_score = 0;
		PROTO_SYNC_RANK *proto = (PROTO_SYNC_RANK*)get_send_data();
		memset(proto->ranks, 0, sizeof(proto->ranks));
		int i = 0;
		for (std::map<uint32_t, std::string>::iterator iter = scm_rank_keys.begin(); iter != scm_rank_keys.end() && i < MAX_RANK_TYPE; ++iter)
		{
			out_rank = 0xffffffff;
			out_score = 0;
			ret = sg_redis_client.zget_rank(iter->second.c_str(), extern_data->player_id, out_rank);
			if (ret == 0)
			{
				out_rank++;
			}
			sg_redis_client.zget_score(iter->second.c_str(), extern_data->player_id, out_score);
			proto->ranks[i].type = iter->first;
			proto->ranks[i].rank = out_rank;
			proto->ranks[i].score = out_score;
			i++;
		}

		fast_send_msg_base(&connecter, extern_data, SERVER_PROTO_RANK_SYNC_RANK, sizeof(PROTO_SYNC_RANK), 0);
	} while(0);
	
	return 0;
}

//更新门宗在排行榜中的分数
static void update_guild_rank_score(uint32_t rank_type, uint32_t guild_id, uint32_t score_old, uint32_t score_new)
{
	char *rank_key = NULL;
	CRedisClient &rc = sg_redis_client;
	rank_key = get_rank_key(rank_type);
	int ret = rc.zset(rank_key, guild_id, score_new);
	if (ret != 0)
	{
		LOG_ERR("[%s:%d] update %s %u failed, score_new:%u, score_old:%u", __FUNCTION__, __LINE__, rank_key, guild_id, score_new, score_old);
	}
}

//将门宗从排行榜中删除
static void del_guild_rank(uint32_t rank_type, uint32_t guild_id)
{
	char *rank_key = NULL;
	CRedisClient &rc = sg_redis_client;
	rank_key = get_rank_key(rank_type);
	std::vector<uint64_t> dels;
	dels.push_back(guild_id);
	int ret = rc.zdel(rank_key, dels);
	if (ret != 0)
	{
		LOG_ERR("[%s:%d] del %s %u failed", __FUNCTION__, __LINE__, rank_key, guild_id);
	}
}

int conn_node_ranksrv::handle_refresh_guild_info(EXTERN_DATA *extern_data)
{
	AutoReleaseBatchRedisGuild t1;
	RedisGuildInfo *req = redis_guild_info__unpack(NULL, get_data_len(), get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] unpack req failed", __FUNCTION__, __LINE__);
		return -1;
	}
	t1.push_back(req);

	LOG_INFO("[%s:%d] guild[%u]", __FUNCTION__, __LINE__, req->guild_id);

	CRedisClient &rc = sg_redis_client;
	char field[128];
	sprintf(field, "%u", req->guild_id);
	uint32_t rank_type = 0;
	uint32_t guild_id = req->guild_id;
	bool bDisband = !(req->master_id > 0 && req->n_member_ids > 0);

	RedisGuildInfo *old_guild = get_redis_guild(guild_id, sg_guild_key, sg_redis_client, t1);

	std::set<uint64_t> player_ids;
	for (size_t i = 0; i < req->n_member_ids; ++i)
	{
		player_ids.insert(req->member_ids[i]);
	}
	std::map<uint64_t, PlayerRedisInfo *> redis_players;
	AutoReleaseBatchRedisPlayer t2;
	get_more_redis_player(player_ids, redis_players, sg_player_key, sg_redis_client, t2);

	req->fc = 0;
	req->exploit = 0;
	for (std::map<uint64_t, PlayerRedisInfo *>::iterator iter = redis_players.begin(); iter != redis_players.end(); ++iter)
	{
		req->fc += iter->second->fighting_capacity;
		req->exploit += iter->second->exploit;
	}

	uint32_t zhenying = req->zhenying;
	uint32_t fc = req->fc;
	uint32_t exploit = req->exploit;
	uint32_t popular = req->popularity;

	uint32_t old_fc = 0;
	uint32_t old_exploit = 0;
	uint32_t old_popular = 0;

	if (old_guild)
	{
		old_fc = old_guild->fc;
		old_exploit = old_guild->exploit;
		old_popular = old_guild->popularity;
	}

	if (bDisband)
	{ //门宗解散，从所有排行榜中删除
		sprintf(field, "%u", guild_id);
		int ret = rc.hdel(sg_guild_key, field);
		if (ret != 0)
		{
			LOG_ERR("[%s:%d] del %s %s failed", __FUNCTION__, __LINE__, sg_guild_key, field);
		}

		rank_type = RANK_GUILD_FC_TOTAL;
		del_guild_rank(rank_type, guild_id);
		rank_type = RANK_GUILD_FC_ZHENYING1 + zhenying - 1;
		del_guild_rank(rank_type, guild_id);

		rank_type = RANK_GUILD_EXPLOIT_TOTAL;
		del_guild_rank(rank_type, guild_id);
		rank_type = RANK_GUILD_EXPLOIT_ZHENYING1 + zhenying - 1;
		del_guild_rank(rank_type, guild_id);

		rank_type = RANK_GUILD_POPULAR_TOTAL;
		del_guild_rank(rank_type, guild_id);
		rank_type = RANK_GUILD_POPULAR_ZHENYING1 + zhenying - 1;
		del_guild_rank(rank_type, guild_id);
	}
	else
	{
		if (fc != old_fc)
		{
			rank_type = RANK_GUILD_FC_TOTAL;
			update_guild_rank_score(rank_type, guild_id, old_fc, fc);

			rank_type = RANK_GUILD_FC_ZHENYING1 + zhenying - 1;
			update_guild_rank_score(rank_type, guild_id, old_fc, fc);
		}
		if (exploit != old_exploit)
		{
			rank_type = RANK_GUILD_EXPLOIT_TOTAL;
			update_guild_rank_score(rank_type, guild_id, old_exploit, exploit);

			rank_type = RANK_GUILD_EXPLOIT_ZHENYING1 + zhenying - 1;
			update_guild_rank_score(rank_type, guild_id, old_exploit, exploit);
		}
		if (popular != old_popular)
		{
			rank_type = RANK_GUILD_POPULAR_TOTAL;
			update_guild_rank_score(rank_type, guild_id, old_popular, popular);

			rank_type = RANK_GUILD_POPULAR_ZHENYING1 + zhenying - 1;
			update_guild_rank_score(rank_type, guild_id, old_popular, popular);
		}

		{
			static uint8_t data_buffer[128 * 1024];
			do
			{
				size_t data_len = redis_guild_info__pack(req, data_buffer);
				if (data_len == (size_t)-1)
				{
					LOG_ERR("[%s:%d] pack redis guild failed, guild[%u]", __FUNCTION__, __LINE__, guild_id);
					break;
				}

				sprintf(field, "%u", guild_id);
				int ret = rc.hset_bin(sg_guild_key, field, (const char *)data_buffer, (int)data_len);
				if (ret < 0)
				{
					LOG_ERR("[%s:%d] set redis guild failed, guild[%u] ret = %d", __FUNCTION__, __LINE__, guild_id, ret);
					break;
				}
				LOG_DEBUG("[%s:%d] save guild[%u] len[%d] ret = %d", __FUNCTION__, __LINE__, guild_id, (int)data_len, ret);
			} while(0);
		}
	}

	return 0;
}

static int on_guild_player_fc_change(uint32_t guild_id)
{
	LOG_INFO("[%s:%d] guild[%u]", __FUNCTION__, __LINE__, guild_id);

	CRedisClient &rc = sg_redis_client;
	char field[128];
	sprintf(field, "%u", guild_id);
	uint32_t rank_type = 0;

	AutoReleaseBatchRedisGuild t2;
	RedisGuildInfo *old_guild = get_redis_guild(guild_id, sg_guild_key, sg_redis_client, t2);
	if (!old_guild)
	{
		return 0;
	}

	uint32_t old_fc = old_guild->fc;
	uint32_t old_exploit = old_guild->exploit;
	std::set<uint64_t> player_ids;
	for (size_t i = 0; i < old_guild->n_member_ids; ++i)
	{
		player_ids.insert(old_guild->member_ids[i]);
	}
	std::map<uint64_t, PlayerRedisInfo *> redis_players;
	AutoReleaseBatchRedisPlayer t1;
	get_more_redis_player(player_ids, redis_players, sg_player_key, sg_redis_client, t1);

	uint32_t fc = 0;
	uint32_t exploit = 0;
	for (std::map<uint64_t, PlayerRedisInfo *>::iterator iter = redis_players.begin(); iter != redis_players.end(); ++iter)
	{
		fc += iter->second->fighting_capacity;
		exploit += iter->second->exploit;
	}

	old_guild->fc = fc;
	old_guild->exploit = exploit;
	uint32_t zhenying = old_guild->zhenying;

	if (fc != old_fc)
	{
		rank_type = RANK_GUILD_FC_TOTAL;
		update_guild_rank_score(rank_type, guild_id, old_fc, fc);

		rank_type = RANK_GUILD_FC_ZHENYING1 + zhenying - 1;
		update_guild_rank_score(rank_type, guild_id, old_fc, fc);
	}
	if (exploit != old_exploit)
	{
		rank_type = RANK_GUILD_EXPLOIT_TOTAL;
		update_guild_rank_score(rank_type, guild_id, old_exploit, exploit);

		rank_type = RANK_GUILD_EXPLOIT_ZHENYING1 + zhenying - 1;
		update_guild_rank_score(rank_type, guild_id, old_exploit, exploit);
	}

	{
		static uint8_t data_buffer[128 * 1024];
		do
		{
			size_t data_len = redis_guild_info__pack(old_guild, data_buffer);
			if (data_len == (size_t)-1)
			{
				LOG_ERR("[%s:%d] pack redis guild failed, guild[%u]", __FUNCTION__, __LINE__, guild_id);
				break;
			}

			sprintf(field, "%u", guild_id);
			int ret = rc.hset_bin(sg_guild_key, field, (const char *)data_buffer, (int)data_len);
			if (ret < 0)
			{
				LOG_ERR("[%s:%d] set redis guild failed, guild[%u] ret = %d", __FUNCTION__, __LINE__, guild_id, ret);
				break;
			}
			LOG_DEBUG("[%s:%d] save guild[%u] len[%d] ret = %d", __FUNCTION__, __LINE__, guild_id, (int)data_len, ret);
		} while(0);
	}

	return 0;
}

int conn_node_ranksrv::handle_refresh_player_world_boss_info(EXTERN_DATA *extern_data)
{
	PlayerWorldBossRedisinfo *req = player_world_boss_redisinfo__unpack(NULL, get_data_len(), get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] player[%lu] unpack req failed", __FUNCTION__, __LINE__, extern_data->player_id);
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
		LOG_ERR("[%s:%d]更新世界boss数据失败，无对应的世界boss,bossid[%lu]", __FUNCTION__, __LINE__, boss_id);
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
			LOG_ERR("[%s:%d] del cur  world boss failed, bossid[%lu] ret = %d", __FUNCTION__, __LINE__, boss_id, ret);
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
			world_boss_info[answer.n_info].head_icon = last_player_redis->head_icon;
			world_boss_info[answer.n_info].level  = last_player_redis->lv; 
			world_boss_info[answer.n_info].zhenying  = last_player_redis->zhenying; 
		}
		PlayerRedisInfo *max_player_redis = get_redis_player(world_boss_info[answer.n_info].max_score_player_id, sg_player_key, sg_redis_client, p1);
		if(max_player_redis != NULL)
		{
			world_boss_info[answer.n_info].max_head_icon = max_player_redis->head_icon;
			world_boss_info[answer.n_info].max_level  = max_player_redis->lv; 
			world_boss_info[answer.n_info].max_zhenying  = max_player_redis->zhenying; 
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
			LOG_ERR("[%s:%d] del cur  world boss failed, bossid[%lu] ret = %d", __FUNCTION__, __LINE__, boss_id, ret);
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
			LOG_ERR("[%s:%d]将本轮排行信息跟新到上轮时，清除本轮榜单信息失败[%lu]", __FUNCTION__, __LINE__, boss_id);
			break;
		}
	}while(0);

	//发奖励(非击杀死亡不给奖励)
	/*if(cur_boss_redis != NULL)
	{
		world_boss_provide_rank_reward(boss_id);
	}*/

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
				LOG_ERR("[%s:%d] 发放玩家排行奖励失败,player_id[%lu] last_time[%lu] now_time[%lu]",
					__FUNCTION__, __LINE__, player_id, last_time, now_time);
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
		uint32_t my_score = 0;
		rc.zget_score(rank_key, player_id, my_score);
		receive_world_boss_reward_to_player(rank, boss_id, player_id, my_score);
		//最后发奖励
	}
	
	return 0;
}

int conn_node_ranksrv::receive_world_boss_reward_to_player(uint32_t rank, uint64_t boss_id, uint64_t player_id, uint32_t score)
{
	WorldBossTable *world_boss_config = get_config_by_id(boss_id, &rank_world_boss_config);
	if(world_boss_config == NULL)
	{
		LOG_ERR("[%s:%d] 世界boss发奖失败,配置错误,player_id[%lu]", __FUNCTION__, __LINE__, player_id);
		return -1;
	}
	std::map<uint32_t, uint32_t> attachs;
	uint32_t drop_id = 0;
	uint32_t mail_id = 0;
	for(uint32_t i = 0; i < world_boss_config->n_Ranking1 && i < world_boss_config->n_Ranking2 && i < world_boss_config->n_Reward && i < world_boss_config->n_MailID; i++)
	{
		if(rank >= world_boss_config->Ranking1[i] && rank < world_boss_config->Ranking2[i])
		{
			drop_id = world_boss_config->Reward[i];
			mail_id = world_boss_config->MailID[i];
			break;
		}
	}
	if(drop_id == 0)
	{
		LOG_ERR("[%s:%d] 世界boss发奖失败 player[%lu] rank[%u]", __FUNCTION__, __LINE__, player_id, rank);
		return -2;
	}

	get_drop_item(drop_id, attachs);

	//邮件发奖
	std::vector<char *> boss_name_vect;
	char boss_name[1024] = {0};
	if(world_boss_config != NULL)
	{
		if(world_boss_config->Type == 1)
		{
			snprintf(boss_name, 1024, "头目-%s", world_boss_config->Name);
		}
		else 
		{
			snprintf(boss_name, 1024, "首领-%s", world_boss_config->Name);
		}
		boss_name_vect.push_back(boss_name);
	}
	send_mail(&connecter, player_id, mail_id , NULL, NULL, NULL, &boss_name_vect, &attachs, MAGIC_TYPE_WORLDBOS_RANK_REWARD);

	//通知玩家弹奖励界面
	RankWorldBossRewardNotify notify;
	RankWorldBossRewardInfo item_info[MAX_WORLD_BOSS_REWARD_ITEM_NUM];
	RankWorldBossRewardInfo *item_info_point[MAX_WORLD_BOSS_REWARD_ITEM_NUM];
	rank_world_boss_reward_notify__init(&notify);
	notify.rank = rank;
	notify.bossid = boss_id;
	notify.score = score;
	
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
			LOG_ERR("[%s:%d] 发放最后一刀奖励失败,player_id[%lu] last_time[%lu] now_time[%lu]",
				__FUNCTION__, __LINE__, player_id, last_time, now_time);
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
		WorldBossTable *world_boss_config = get_config_by_id(boss_id, &rank_world_boss_config);
		std::vector<char *> boss_name_vect;
		char boss_name[1024] = {0};
		if(world_boss_config != NULL)
		{
			if(world_boss_config->Type == 1)
			{
				snprintf(boss_name, 1024, "头目-%s", world_boss_config->Name);
			}
			else 
			{
				snprintf(boss_name, 1024, "首领-%s", world_boss_config->Name);
			}
			boss_name_vect.push_back(boss_name);
		}
		send_mail(&connecter, player_id, 270300041, NULL, NULL, NULL, &boss_name_vect, &attachs, MAGIC_TYPE_WORLDBOS_KILL_REWARD);
	}
		
	return 0;
}
