#include "rank_util.h"

AutoReleaseRankRedisInfo::AutoReleaseRankRedisInfo()
{
	
	cur_world_boss = NULL;
	befor_world_boss = NULL;
	world_boss_reward = NULL; 

}


AutoReleaseRankRedisInfo::~AutoReleaseRankRedisInfo()
{

	if(cur_world_boss)
		cur_world_boss_redisinfo__free_unpacked(cur_world_boss, NULL);

	if(befor_world_boss)
		befor_world_boss_redisinfo__free_unpacked(befor_world_boss, NULL);

	if(world_boss_reward)
		world_boss_receive_reward_info__free_unpacked(world_boss_reward, NULL);		


}


void AutoReleaseRankRedisInfo::set_cur_world_boss(CurWorldBossRedisinfo* r)
{

	if (cur_world_boss)
	{
		cur_world_boss_redisinfo__free_unpacked(cur_world_boss, NULL);		
	}
	cur_world_boss  = r;
}


void AutoReleaseRankRedisInfo::set_befor_world_boss(BeforWorldBossRedisinfo* r)
{

	if (befor_world_boss)
	{
		befor_world_boss_redisinfo__free_unpacked(befor_world_boss, NULL);		
	}
	befor_world_boss = r;
}


void AutoReleaseRankRedisInfo::set_world_boss_reward(WorldBossReceiveRewardInfo* r)
{

	if (world_boss_reward)
	{
		world_boss_receive_reward_info__free_unpacked(world_boss_reward, NULL);		
	}
	world_boss_reward = r;

}


CurWorldBossRedisinfo *get_redis_cur_world_boss(uint64_t boss_id, char *world_boss_key, CRedisClient &rc, AutoReleaseRankRedisInfo &_pool)
{
	static uint8_t data_buffer[32 * 1024];
	int data_len = sizeof(data_buffer);
	char field[64];
	sprintf(field, "%lu", boss_id);
	int ret = rc.hget_bin(world_boss_key, field, (char *)data_buffer, &data_len);
	if (ret == 0)
	{
		CurWorldBossRedisinfo *ret = cur_world_boss_redisinfo__unpack(NULL, data_len, data_buffer);
		if(ret)
			_pool.set_cur_world_boss(ret);
		return ret;
	}

	return NULL;
}
BeforWorldBossRedisinfo *get_redis_befor_world_boss(uint64_t boss_id, char *world_boss_key, CRedisClient &rc, AutoReleaseRankRedisInfo &_pool)
{
	static uint8_t data_buffer[32 * 1024];
	int data_len = sizeof(data_buffer);
	char field[64];
	sprintf(field, "%lu", boss_id);
	int ret = rc.hget_bin(world_boss_key, field, (char *)data_buffer, &data_len);
	if (ret == 0)
	{
		BeforWorldBossRedisinfo *ret = befor_world_boss_redisinfo__unpack(NULL, data_len, data_buffer);
		if(ret)
			_pool.set_befor_world_boss(ret);
		return ret;
	}

	return NULL;
}

WorldBossReceiveRewardInfo *get_redis_world_boss_receive_reward_info(uint64_t player_id, char *world_boss_key, CRedisClient &rc, AutoReleaseRankRedisInfo &_pool)
{
	static uint8_t data_buffer[32 * 1024];
	int data_len = sizeof(data_buffer);
	char field[64];
	sprintf(field, "%lu", player_id);
	int ret = rc.hget_bin(world_boss_key, field, (char *)data_buffer, &data_len);
	if (ret == 0)
	{
		WorldBossReceiveRewardInfo *ret = world_boss_receive_reward_info__unpack(NULL, data_len, data_buffer);
		if(ret)
			_pool.set_world_boss_reward(ret);
		return ret;
	}

	return NULL;
}
