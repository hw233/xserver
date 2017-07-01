#ifndef MONSTER_AI_NORMAL_H
#define MONSTER_AI_NORMAL_H

#include "monster.h"
#include "player.h"
#include <stdint.h>

extern struct ai_interface monster_ai_normal_interface;
extern struct ai_interface monster_ai_circle_interface;
extern struct ai_interface monster_ai_0_interface;
extern struct ai_interface monster_ai_4_interface;
extern struct ai_interface monster_ai_5_interface;
extern struct ai_interface monster_ai_7_interface;
extern struct ai_interface monster_ai_8_interface;
extern struct ai_interface monster_ai_10_interface;
extern struct ai_interface monster_ai_11_interface;
extern struct ai_interface monster_ai_12_interface;
extern struct ai_interface monster_ai_13_interface;
extern struct ai_interface monster_ai_14_interface;
extern struct ai_interface monster_ai_15_interface;
extern struct ai_interface monster_ai_16_interface;
extern struct ai_interface monster_ai_17_interface;
extern struct ai_interface monster_ai_18_interface;
extern struct ai_interface monster_ai_19_interface;
extern struct ai_interface monster_ai_20_interface;
extern struct ai_interface monster_ai_21_interface;

int get_monster_hp_percent(monster_struct *monster);
//计算技能硬直时间
uint32_t count_skill_delay_time(struct SkillTable *config);
//uint32_t choose_rand_skill(monster_struct *monster);
uint32_t choose_first_skill(monster_struct *monster);
uint32_t choose_skill_and_add_cd(monster_struct *monster);
//void send_patrol_move(monster_struct *monster);
void monster_hit_notify_to_many_player(uint64_t skill_id, monster_struct *monster, player_struct *owner, std::vector<unit_struct *> *target);
void monster_hit_notify_to_player(uint64_t skill_id, monster_struct *monster, unit_struct *player);
void monster_cast_skill_to_player(uint64_t skill_id, monster_struct *monster, unit_struct *player, bool use_target_pos);
void monster_cast_call_monster_skill(monster_struct *monster, uint64_t skill_id);
void monster_cast_skill_to_direct(uint64_t skill_id, monster_struct *monster, float direct_x, float direct_z);
void monster_cast_skill_to_pos(uint64_t skill_id, monster_struct *monster, float pos_x, float pos_z);
void monster_cast_immediate_skill_to_player(uint64_t skill_id, monster_struct *monster, player_struct *owner, unit_struct *player);
//void monster_cast_delay_skill_to_player(uint64_t skill_id, monster_struct *monster, unit_struct *player);
bool check_monster_relive(monster_struct *monster);
void do_normal_patrol(monster_struct *monster);
void do_normal_attack(monster_struct *monster);
void do_normal_pursue(monster_struct *monster);
#endif /* MONSTER_AI_NORMAL_H */
