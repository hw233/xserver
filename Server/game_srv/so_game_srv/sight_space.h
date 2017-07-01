#ifndef SIGHT_SPACE_H
#define SIGHT_SPACE_H

#include "player.h"
#include "monster.h"

#define MAX_PLAYER_IN_SIGHT_SPACE 2
#define MAX_MONSTER_IN_SIGHT_SPACE 30

struct sight_space_data
{
	bool mark_delete;
	uint64_t player_id[MAX_PLAYER_IN_SIGHT_SPACE];
	uint64_t monster_uuid[MAX_MONSTER_IN_SIGHT_SPACE];
	uint64_t task_event[MAX_MONSTER_IN_SIGHT_SPACE];
	uint64_t n_monster_uuid;
	uint64_t type; //0 ÈÎÎñ 1Ñ°±¦ 2ÑºïÚ
};

class sight_space_struct
{
public:
	sight_space_struct();
	~sight_space_struct();
	int broadcast_player_create(player_struct *player);
	int broadcast_monster_create(monster_struct *monster);
	
	int broadcast_player_delete(player_struct *player);
	int broadcast_monster_delete(monster_struct *monster);
	
	
	bool is_task_event_exist(uint64_t event_id);
	int insert_task_event(uint64_t event_id);
	
//	struct position players_prev_pos[MAX_PLAYER_IN_SIGHT_SPACE];
	player_struct *players[MAX_PLAYER_IN_SIGHT_SPACE];
	monster_struct *monsters[MAX_MONSTER_IN_SIGHT_SPACE];

	sight_space_data *data;
};

#endif /* SIGHT_SPACE_H */
