#ifndef _FRIEND_CONFIG_H__
#define _FRIEND_CONFIG_H__

#include <map>
#include <vector>
#include <set>
#include <stdint.h>
#include "excel_data.h"
#include "lua_template.h"

extern uint32_t sg_friend_recent_num;
extern uint32_t sg_friend_contact_num;
extern uint32_t sg_friend_contact_extend_num;
extern uint32_t sg_friend_block_num;
extern uint32_t sg_friend_enemy_num;
extern uint32_t sg_friend_apply_num;
extern uint32_t sg_friend_gift_send_num;
extern uint32_t sg_friend_gift_accept_num;
extern uint32_t sg_friend_group_num;
extern std::set<uint32_t> sg_friend_gift_id;
extern uint32_t sg_friend_track_item_id;
extern uint32_t sg_friend_track_item_num;
extern uint32_t sg_friend_track_time;
extern uint32_t  MAX_TOWER_LEVEL;

extern std::map<uint64_t, struct RandomCardRewardTable*> wanyaoka_reward_config; //万妖卡奖励配置
extern std::map<uint64_t, struct ParameterTable *> parameter_config;
extern std::map<uint64_t, struct CampTable*> zhenying_base_config; //阵营基础信息表
extern std::map<uint64_t, struct GiftTable*> friend_gift_config; //好友礼物表
extern std::map<uint64_t, struct P20076Table*> tower_level_config; //冲塔表

int read_all_excel_data();

#endif
