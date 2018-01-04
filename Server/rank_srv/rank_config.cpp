#include "rank_config.h"
#include "sproto.h"
#include "sprotoc_common.h"
#include "lua_load.h"
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>
#include <fcntl.h>
extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}
#include "excel_data.h"

typedef std::map<uint64_t, void *> *config_type;
std::map<uint64_t, struct WorldBossTable*> rank_world_boss_config; //世界boss表
std::map<uint64_t, struct ActorAttributeTable *> actor_attribute_config;
std::map<uint64_t, struct MonsterTable *> monster_config;
std::map<uint64_t, struct WorldBossRewardTable *> world_boss_reward_config;
std::map<uint64_t, struct ParameterTable *> parameter_config;
std::map<uint64_t, struct RankingRewardTable *> rank_reward_config;//排行榜奖励表
std::map<uint64_t, std::vector<struct RankingRewardTable *> > rank_reward_map;//排行榜奖励表

uint32_t sg_rank_reward_time;
uint32_t sg_rank_reward_interval;
uint32_t sg_rank_reward_first_interval;

static void generate_parameters(void)
{
	ParameterTable *config = NULL;
	config = get_config_by_id(161001016, &parameter_config);
	if (config && config->n_parameter1 >= 3)
	{
		sg_rank_reward_time = config->parameter1[0];
		sg_rank_reward_interval = config->parameter1[1];
		sg_rank_reward_first_interval = config->parameter1[2];
	}
}

static void adjust_rank_reward_config(void)
{
	for (std::map<uint64_t, std::vector<struct RankingRewardTable *> >::iterator iter = rank_reward_map.begin(); iter != rank_reward_map.end(); ++iter)
	{
		iter->second.clear();
	}
	rank_reward_map.clear();
	for (std::map<uint64_t, struct RankingRewardTable *>::iterator iter = rank_reward_config.begin(); iter != rank_reward_config.end(); ++iter)
	{
		RankingRewardTable *config = iter->second;
		assert(config->n_Reward == config->n_RewardNum);
		assert(config->n_RankingTableId == config->n_RankingTableIdName);
		for (uint32_t i = 0; i < config->n_RankingTableId; ++i)
		{
			rank_reward_map[config->RankingTableId[i]].push_back(config);
		}
	}
}

#define READ_SPB_MAX_LEN (1024 * 1024)
int read_all_rank_excel_data()
{
	char *buf = (char *)malloc(READ_SPB_MAX_LEN);
	if (!buf)
		return -1;
	int fd = open("../excel_data/1.spb", O_RDONLY);
	if (fd <= 0) {
		return (-1);
	}
	size_t size =  read(fd, buf, READ_SPB_MAX_LEN);
	struct sproto *sp = sproto_create(&buf[0], size);
	close(fd);
    lua_State *L = luaL_newstate();
	luaL_openlibs(L);

	int ret;
	struct sproto_type *type = sproto_type(sp, "WorldBossTable");
	assert(type);
	ret = traverse_main_table(L, type, "../lua_data/WorldBossTable.lua", (config_type)&rank_world_boss_config);
	assert(ret == 0);

	type = sproto_type(sp, "ActorAttributeTable");
	assert(type);	
	ret = traverse_main_table(L, type, "../lua_data/ActorAttributeTable.lua", (config_type)&actor_attribute_config);
	assert(ret == 0);

	type = sproto_type(sp, "MonsterTable");
	assert(type);		
	ret = traverse_main_table(L, type, "../lua_data/MonsterTable.lua", (config_type)&monster_config);
	assert(ret == 0);

	type = sproto_type(sp, "WorldBossRewardTable");
	assert(type);		
	ret = traverse_main_table(L, type, "../lua_data/WorldBossRewardTable.lua", (config_type)&world_boss_reward_config);
	assert(ret == 0);

	type = sproto_type(sp, "ParameterTable");
	assert(type);		
	ret = traverse_main_table(L, type, "../lua_data/ParameterTable.lua", (config_type)&parameter_config);
	assert(ret == 0);

	type = sproto_type(sp, "RankingRewardTable");
	assert(type);		
	ret = traverse_main_table(L, type, "../lua_data/RankingRewardTable.lua", (config_type)&rank_reward_config);
	assert(ret == 0);

	generate_parameters();
	adjust_rank_reward_config();

	lua_close(L);	
	sproto_release(sp);
	free(buf);
	return (0);
}



