#ifndef REDIS_UTIL_H
#define REDIS_UTIL_H
#include <vector>
#include <set>
#include <map>
#include "player_redis_info.pb-c.h"
#include "redis_client.h"

class AutoReleaseRedisPlayer
{
public:
	AutoReleaseRedisPlayer();
	~AutoReleaseRedisPlayer();
	void set(PlayerRedisInfo* r);
	void set_world_boss(CurWorldBossRedisinfo* r);
	void set_world_boss_reward(WorldBossReceiveRewardInfo* r);
private:
	PlayerRedisInfo *pointer;
	CurWorldBossRedisinfo *world_boss_pointer;
	WorldBossReceiveRewardInfo *world_boss_reward; 
};

class AutoReleaseBatchRedisPlayer
{
public:
	AutoReleaseBatchRedisPlayer();
	~AutoReleaseBatchRedisPlayer();

	void push_back(PlayerRedisInfo *player);
private:
	std::vector<PlayerRedisInfo *> pointer_vec;
};

PlayerRedisInfo *get_redis_player(uint64_t player_id, char *player_key, CRedisClient &rc, AutoReleaseRedisPlayer &_pool);
PlayerRedisInfo *get_redis_player(uint64_t player_id, char *player_key, CRedisClient &rc, AutoReleaseBatchRedisPlayer &_pool);
int get_more_redis_player(std::set<uint64_t> &player_ids, std::map<uint64_t, PlayerRedisInfo*> &redis_players,
	char *player_key, CRedisClient &rc, AutoReleaseBatchRedisPlayer &_pool);
CurWorldBossRedisinfo *get_redis_cur_world_boss(uint64_t boss_id, char *world_boss_key, CRedisClient &rc, AutoReleaseRedisPlayer &_pool);
WorldBossReceiveRewardInfo *get_redis_world_boss_receive_reward_info(uint64_t player_id, char *world_boss_key, CRedisClient &rc, AutoReleaseRedisPlayer &_pool);

#endif /* REDIS_UTIL_H */

