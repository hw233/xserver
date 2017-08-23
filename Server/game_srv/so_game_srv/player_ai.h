#ifndef PLAYER_AI_H
#define PLAYER_AI_H

#include <stdint.h>
#include "player.h"

void reset_patrol_index(player_struct *player);
uint32_t choose_rand_skill(player_struct *player);
void find_next_position(player_struct *player);
void pvp_player_ai_send_move(player_struct *player);
void do_ai_player_attack(player_struct *monster, struct ai_player_data *ai_player_data, player_struct **enemy, int enemy_num);
bool do_attack(player_struct *player, struct ai_player_data *ai_player_data, player_struct *target, uint32_t skill_id);

#endif /* PLAYER_AI_H */
