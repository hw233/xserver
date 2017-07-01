#ifndef SIGHT_SPACE_MANAGER_H
#define SIGHT_SPACE_MANAGER_H

#include "sight_space.h"
#include "mem_pool.h"

extern comm_pool sight_space_manager_sight_space_data_pool;
extern std::vector<sight_space_struct *> sight_space_manager_mark_delete_sight_space;

class sight_space_manager
{
public:
	static void on_tick();
	static int init_sight_space(int num, unsigned long key);
	static sight_space_struct *create_sight_space(player_struct *player);
//	static int add_player_to_sight_space(sight_space_struct *sight_space, player_struct *player);
	static int del_player_from_sight_space(sight_space_struct *sight_space, player_struct *player, bool enter_scene);
	static int get_sight_space_num();
	static int get_sight_space_pool_max_num();	
	static void mark_sight_space_delete(sight_space_struct *sight_space);
};


#endif /* SIGHT_SPACE_MANAGER_H */
