#ifndef PVP_MATCH_MANAGER_H
#define PVP_MATCH_MANAGER_H

#include <stdint.h>
#include "player.h"

#define PVP_MATCH_PLAYER_NUM_3 (3)
#define PVP_MATCH_PLAYER_NUM_5 (5)

void pvp_match_manager_on_tick();

void pvp_match_on_player_offline(player_struct *player);  //玩家离线
void pvp_match_on_team_member_changed(player_struct *player);  //队伍人员变更, 队长变更，加减人，队伍解散

uint32_t pvp_match_is_player_in_cd(player_struct *player);  //返回0表示无CD，否则返回CD时间
bool pvp_match_is_player_in_waiting(uint64_t player_id);
bool pvp_match_is_team_in_waiting(uint64_t id);

int pvp_match_add_player_to_waiting(player_struct *player, int type);
int pvp_match_add_team_to_waiting(player_struct *player, int type);

int pvp_match_del_player_from_waiting(player_struct *player);
int pvp_match_del_team_from_waiting(uint64_t id);

int pvp_match_player_set_ready(player_struct *player);
int	pvp_match_player_cancel(player_struct *player);
int pvp_match_player_praise(player_struct *player, uint64_t target_id);

//int pvp_raid_get_player_index(raid_struct *raid, player_struct *player);
//int pvp_raid_get_player_index2(raid_struct *raid, uint64_t player_id);

int pvp_match_single_ai_player_3(player_struct *player);
int pvp_match_single_ai_player_5(player_struct *player);

//获取复活点坐标
void pvp_raid_get_relive_pos(raid_struct *raid, int32_t *ret_pos_x, int32_t *ret_pos_z, int32_t *ret_direct);

/* #define MATCH_LEVEL_DIFF (get_config_by_id(161000055, &parameter_config)->parameter1[0]) */
/* #define TEAM_LEVEL1_DIFF_L (get_config_by_id(161000056, &parameter_config)->parameter1[0]) */
/* #define TEAM_LEVEL1_DIFF_R (get_config_by_id(161000056, &parameter_config)->parameter1[1]) */
/* #define TEAM_LEVEL2_DIFF_L (get_config_by_id(161000057, &parameter_config)->parameter1[0]) */
/* #define TEAM_LEVEL2_DIFF_R (get_config_by_id(161000057, &parameter_config)->parameter1[1]) */

#endif /* PVP_MATCH_MANAGER_H */
