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
std::map<uint64_t, struct ParameterTable *> parameter_config;
std::map<uint64_t, struct RankingRewardTable *> rank_reward_config;//排行榜奖励表
std::map<uint64_t, std::vector<struct RankingRewardTable *> > rank_reward_map;//排行榜奖励表
std::map<uint64_t, struct DropConfigTable *> drop_config;

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

	type = sproto_type(sp, "ParameterTable");
	assert(type);		
	ret = traverse_main_table(L, type, "../lua_data/ParameterTable.lua", (config_type)&parameter_config);
	assert(ret == 0);

	type = sproto_type(sp, "RankingRewardTable");
	assert(type);		
	ret = traverse_main_table(L, type, "../lua_data/RankingRewardTable.lua", (config_type)&rank_reward_config);
	assert(ret == 0);

	type = sproto_type(sp, "DropConfigTable");
	assert(type);		
	ret = traverse_main_table(L, type, "../lua_data/DropConfigTable.lua", (config_type)&drop_config);
	assert(ret == 0);

	generate_parameters();
	adjust_rank_reward_config();

	lua_close(L);	
	sproto_release(sp);
	free(buf);
	return (0);
}

void add_drop_item(uint32_t item_id, uint32_t num_max, uint32_t num_min, std::map<uint32_t, uint32_t> &item_list, uint32_t stack)
{
	if (item_id > 200000000 && item_id < 209999999)
	{
		uint32_t rand_num = rand_between(num_min, num_max);
		if (rand_num > 0)
		{
			item_list[item_id] += rand_num;
		}
	}
	else
	{
		get_drop_item(item_id, item_list, stack + 1);
	}
}

int get_drop_item(uint32_t drop_id, std::map<uint32_t, uint32_t> &item_list, uint32_t stack)
{
	if (stack >= 10)
	{
		LOG_ERR("[%s:%d] stack level too deep, drop_id:%u", __FUNCTION__, __LINE__, drop_id);
		return -1;
	}

	DropConfigTable *config = get_config_by_id(drop_id, &drop_config);
	if (!config)
	{
		LOG_ERR("[%s:%d] get drop config failed, drop_id:%u", __FUNCTION__, __LINE__, drop_id);
		return -1;
	}

	if (config->ProType == 0) //统一概率
	{
		uint64_t total_prob = 0;
		for (uint32_t i = 0; i < config->n_DropID && i < config->n_Probability && i < config->n_NumMin && i < config->n_NumMax; ++i)
		{
			total_prob += config->Probability[i];
		}

		if (total_prob == 0)
		{
			LOG_ERR("[%s:%d] total_probability is 0, drop_id:%u", __FUNCTION__, __LINE__, drop_id);
			return -1;
		}

		uint32_t rand_val = random() % total_prob;
		uint32_t add_val = 0;
		for (uint32_t i = 0; i < config->n_DropID && i < config->n_Probability && i < config->n_NumMin && i < config->n_NumMax; ++i)
		{
			if (add_val <= rand_val && rand_val < add_val + config->Probability[i])
			{
				add_drop_item(config->DropID[i], config->NumMax[i], config->NumMin[i], item_list, stack);
				break;
			}
			else
			{
				add_val += config->Probability[i];
			}
		}
	}
	else //单独概率
	{
		for (uint32_t i = 0; i < config->n_DropID && i < config->n_Probability && i < config->n_NumMin && i < config->n_NumMax; ++i)
		{
			uint32_t rand_val = random() % RAND_RATE_BASE;
			if (rand_val < config->Probability[i])
			{
				add_drop_item(config->DropID[i], config->NumMax[i], config->NumMin[i], item_list, stack);
			}
		}
	}

	return 0;
}


