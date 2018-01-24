#include "doufachang_util.h"
#include "conn_node_doufachangsrv.h"
#include "doufachang_config.h"
#include "time_helper.h"
#include <stdint.h>

uint64_t sg_next_copy_rank_time;
void reinit_default_doufachang_info(PlayerDoufachangInfo *info)
{
	info->challenge_count = DEFAULT_CHALLENGE_COUNT;
	info->next_add_count = 0;
	info->max_rank = 9999;
}

AutoReleaseDoufachangInfo::AutoReleaseDoufachangInfo()
{
	pointer = NULL;
}

AutoReleaseDoufachangInfo::~AutoReleaseDoufachangInfo()
{
	if (pointer)	
		player_doufachang_info__free_unpacked(pointer, NULL);
}
void AutoReleaseDoufachangInfo::set(PlayerDoufachangInfo* r)
{
	if (pointer)
	{
		player_doufachang_info__free_unpacked(pointer, NULL);		
	}
	pointer = r;
}
AutoReleaseDoufachangRecord::AutoReleaseDoufachangRecord()
{
	pointer = NULL;
}

AutoReleaseDoufachangRecord::~AutoReleaseDoufachangRecord()
{
	if (pointer)	
		doufachang_record_answer__free_unpacked(pointer, NULL);
}
void AutoReleaseDoufachangRecord::set(DoufachangRecordAnswer * r)
{
	if (pointer)
	{
		doufachang_record_answer__free_unpacked(pointer, NULL);		
	}
	pointer = r;
}

static uint8_t data_buffer[1024];
DoufachangRecordAnswer *get_player_doufachang_record(uint64_t player_id, char *player_key, CRedisClient &rc, AutoReleaseDoufachangRecord &_pool)
{
	int data_len = sizeof(data_buffer);
	char field[64];
	sprintf(field, "%lu", player_id);
	int ret = rc.hget_bin(player_key, field, (char *)data_buffer, &data_len);
	if (ret == 0)
	{
		DoufachangRecordAnswer *ret = doufachang_record_answer__unpack(NULL, data_len, data_buffer);
		_pool.set(ret);
		return ret;
	}
	return NULL;
}
int save_player_doufachang_record(DoufachangRecordAnswer *info, uint64_t player_id, char *player_key, CRedisClient &rc)
{
	assert(info);
	size_t size = doufachang_record_answer__pack(info, data_buffer);
	if (size <= 0 || size >= sizeof(data_buffer))
	{
		LOG_ERR("%s: pack %lu failed, size = %zu", __FUNCTION__, player_id, size);
		return (-1);
	}
	char field[64];
	sprintf(field, "%lu", player_id);
	int ret = rc.hset_bin(player_key, field, (char *)data_buffer, size);
	if (ret != 1 && ret != 0)
	{
		LOG_ERR("%s: hset %lu failed, size = %zu, ret = %d", __FUNCTION__, player_id, size, ret);
		return (-1);		
	}
	return (0);
}

PlayerDoufachangInfo *get_player_doufachang_info(uint64_t player_id, char *player_key, CRedisClient &rc, AutoReleaseDoufachangInfo &_pool)
{
	int data_len = sizeof(data_buffer);
	char field[64];
	sprintf(field, "%lu", player_id);
	int ret = rc.hget_bin(player_key, field, (char *)data_buffer, &data_len);
	if (ret == 0)
	{
		PlayerDoufachangInfo *ret = player_doufachang_info__unpack(NULL, data_len, data_buffer);
		_pool.set(ret);
		return ret;
	}

	return NULL;
}

int save_player_doufachang_info(PlayerDoufachangInfo *info, uint64_t player_id, char *player_key, CRedisClient &rc)
{
	assert(info);
	size_t size = player_doufachang_info__pack(info, data_buffer);
	if (size <= 0 || size >= sizeof(data_buffer))
	{
		LOG_ERR("%s: pack %lu failed, size = %zu", __FUNCTION__, player_id, size);
		return (-1);
	}
	char field[64];
	sprintf(field, "%lu", player_id);
	int ret = rc.hset_bin(player_key, field, (char *)data_buffer, size);
	if (ret != 1 && ret != 0)
	{
		LOG_ERR("%s: hset %lu failed, size = %zu, ret = %d", __FUNCTION__, player_id, size, ret);
		return (-1);		
	}
	return (0);
}

void copy_doufachang_rank(char *from, char *to, CRedisClient &rc)
{
	rc.del(to);
	
	CAutoRedisReply autoR;
	redisReply *r = rc.hgetall_bin(from, autoR);
	if (r == NULL || r->type != REDIS_REPLY_ARRAY)
	{
		LOG_ERR("[%s:%d] get redis failed", __FUNCTION__, __LINE__);
		return ;
	}
	for (size_t i = 0; i + 1 < r->elements; i = i + 2)
	{
		struct redisReply *field = r->element[i];
		struct redisReply *value = r->element[i+1];
		if (field->type != REDIS_REPLY_STRING || value->type != REDIS_REPLY_STRING)
			continue;
		int ret = rc.hset_bin(to, field->str, value->str, value->len);
		if (ret != 1 && ret != 0)
		{
			LOG_ERR("%s: hsetbin failed[%d]", __FUNCTION__, ret);
		}
	}	
}

// void init_doufachang_player_info(uint64_t player_id, PlayerDoufachangInfo *info, uint32_t now)
// {
// 	player_doufachang_info__init(info);
// 	info->challenge_count = DEFAULT_CHALLENGE_COUNT;
// 	info->next_add_count = now;
// 	info->refresh_buy_count = time_helper::nextOffsetTime(5 * 3600, now);;
// }

static uint32_t get_player_reward_id(uint64_t player_id)
{
	uint64_t rank;
	if (sg_redis_client.mget_uint64(conn_node_doufachangsrv::doufachang_rank_reward_key, 1, &player_id, &rank) != 0
		|| rank == 0)
	{
		return (0);
	}

	for (std::map<uint64_t, struct ArenaRewardTable*>::iterator ite = doufachang_reward_config.begin();
		 ite != doufachang_reward_config.end(); ++ite)
	{
		struct ArenaRewardTable *config = ite->second;
		if (rank <= config->Low && rank >= config->Max)
		{
			return config->ID;
		}
	}
	return (0);
}

bool update_doufachang_player_info(uint64_t player_id, PlayerDoufachangInfo *info, uint32_t now)
{
	int ret = false;
	if (!info)
		return false;
		//挑战次数
	if (now >= info->next_add_count)
	{
		int num = (now - info->next_add_count) / ADD_COUNT_SECONDS + 1;
		if (info->challenge_count < DEFAULT_CHALLENGE_COUNT)
		{
			info->challenge_count += num;
			if (info->challenge_count > DEFAULT_CHALLENGE_COUNT)
				info->challenge_count = DEFAULT_CHALLENGE_COUNT;
		}
		
		info->next_add_count += num * ADD_COUNT_SECONDS;
		if (num > 0)
			ret = true;
	}

		//购买挑战次数
	uint32_t refresh = time_helper::nextOffsetTime(5 * 3600, now);
	if (now >= info->refresh_buy_count)
	{
		info->buy_count = 0;
		info->refresh_buy_count = refresh;
		ret = true;
	}

		//排名奖励
	if (now >= info->refresh_reward_id && now < sg_next_copy_rank_time)
	{
		info->reward_id = get_player_reward_id(player_id);
		info->refresh_reward_id = sg_next_copy_rank_time;
		ret = true;
	}
	return ret;
}

void sync_doufachang_rank_to_gamesrv(uint64_t player_id, uint32_t player_rank, uint64_t defencer_id, uint32_t defencer_rank)
{
	DOUFACHANG_SYNC_RANK *req = (DOUFACHANG_SYNC_RANK *)conn_node_doufachangsrv::get_send_data();
	uint32_t data_len = sizeof(DOUFACHANG_SYNC_RANK);
	memset(req, 0, data_len);
	req->my_rank = player_rank;
	req->defencer_id = defencer_id;
	req->defencer_rank = defencer_rank;
	EXTERN_DATA ext_data;
	ext_data.player_id = player_id;
	fast_send_msg_base(conn_node_doufachangsrv::instance(), &ext_data, SERVER_PROTO_DOUFACHANG_SYNC_RANK, data_len, 0);
}

