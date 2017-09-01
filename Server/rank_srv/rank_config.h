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
extern std::map<uint64_t, struct WorldBossRewardTable *> world_boss_reward_config; //世界boss排行奖励表
extern std::map<uint64_t, struct ParameterTable *> parameter_config;//参数表
int read_all_rank_excel_data();

#endif
