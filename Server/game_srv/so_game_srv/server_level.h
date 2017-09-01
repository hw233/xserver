#ifndef __SERVER_LEVEL_H__
#define __SERVER_LEVEL_H__

#include "global_shared_data.h"

class player_struct;

void load_server_level_info(void);
void save_server_level_info(void);
void break_server_level(void);
void check_server_level(void);
void mark_server_level_break(player_struct *player);
void server_level_listen_raid_finish(uint32_t raid_id, player_struct *player);
void broadcast_server_level_info(void);

bool is_server_level_limit(uint32_t player_level);
double get_server_exp_addition(uint32_t player_level);
ServerLevelTable *get_server_level_config(void);

#endif
