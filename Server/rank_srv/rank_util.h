#ifndef __RANK_UTIL_H__
#define __RANK_UTIL_H__
#include "player_redis_info.pb-c.h"
#include "redis_client.h"
class AutoReleaseRankRedisInfo
{
public:
	AutoReleaseRankRedisInfo();
	~AutoReleaseRankRedisInfo();
	void set_cur_world_boss(CurWorldBossRedisinfo* r);
	void set_befor_world_boss(BeforWorldBossRedisinfo* r);
	void set_world_boss_reward(WorldBossReceiveRewardInfo* r);
private:
	CurWorldBossRedisinfo *cur_world_boss;
	BeforWorldBossRedisinfo *befor_world_boss;
	WorldBossReceiveRewardInfo *world_boss_reward; 
};

CurWorldBossRedisinfo *get_redis_cur_world_boss(uint64_t boss_id, char *world_boss_key, CRedisClient &rc, AutoReleaseRankRedisInfo &_pool);

BeforWorldBossRedisinfo *get_redis_befor_world_boss(uint64_t boss_id, char *world_boss_key, CRedisClient &rc, AutoReleaseRankRedisInfo &_pool);

WorldBossReceiveRewardInfo *get_redis_world_boss_receive_reward_info(uint64_t player_id, char *world_boss_key, CRedisClient &rc, AutoReleaseRankRedisInfo &_pool);
#endif
