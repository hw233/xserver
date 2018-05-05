#ifndef _RANK_CONFIG_H__
#define _RANK_CONFIG_H__

#include <map>
#include <vector>
#include <set>
#include <stdint.h>
#include "excel_data.h"
#include "lua_template.h"
#include "game_event.h"


extern std::map<uint64_t, struct WorldBossTable*> rank_world_boss_config; //世界boss表
extern std::map<uint64_t, struct ActorAttributeTable *> actor_attribute_config;//属性表
extern std::map<uint64_t, struct MonsterTable *> monster_config;//怪物表
extern std::map<uint64_t, struct ParameterTable *> parameter_config;//参数表
extern std::map<uint64_t, struct RankingRewardTable *> rank_reward_config;//排行榜奖励表
extern std::map<uint64_t, std::vector<struct RankingRewardTable *> > rank_reward_map;//排行榜奖励表
extern std::map<uint64_t, struct DropConfigTable *> drop_config;

extern uint32_t sg_rank_reward_time;
extern uint32_t sg_rank_reward_interval;
extern uint32_t sg_rank_reward_first_interval;

int read_all_rank_excel_data();
void add_drop_item(uint32_t item_id, uint32_t num_max, uint32_t num_min, std::map<uint32_t, uint32_t> &item_list, uint32_t stack);
int get_drop_item(uint32_t drop_id, std::map<uint32_t, uint32_t> &item_list, uint32_t stack = 0);

#endif
