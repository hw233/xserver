#ifndef _ACTIVITY_CONFIG_H__
#define _ACTIVITY_CONFIG_H__

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
extern std::map<uint64_t, struct ServerResTable *> server_res_config; //服务器资源配置
extern std::map<uint64_t, struct LimitActivityControlTable *> time_limit_control_config; //限时活动表
extern std::map<uint64_t, std::vector<struct PowerMasterTable *> > zhanlidaren_batch_config; //战力达人批次表
extern std::map<uint64_t, std::vector<struct Top10GangsTable *> > shidamenzong_batch_config; //十大门宗批次表

int read_all_activity_excel_data();

PowerMasterTable *get_zhanlidaren_config(uint32_t activity_id, uint32_t gift_id);

#endif
