//战斗结束后跟随玩家
#include <math.h>
#include <stdlib.h>
#include "game_event.h"
#include "monster_ai.h"
#include "monster_manager.h"
#include "path_algorithm.h"
#include "player_manager.h"
#include "time_helper.h"
#include "unit.h"
#include "collect.h"
#include "check_range.h"
#include "cached_hit_effect.h"
#include "camp_judge.h"
#include "count_skill_damage.h"
#include "msgid.h"
#include "buff.h"
#include "buff_manager.h"

static void ai_dead_17(monster_struct *monster, scene_struct *scene)
{
	monster->ai_state = AI_DEAD_STATE;
}

static struct position *get_sight_player_pos(monster_struct *monster)
{
	for (int i = 0; i < monster->data->cur_sight_player; ++i)
	{
		player_struct *player = player_manager::get_player_by_id(monster->data->sight_player[i]);
		if (!player)
			continue;
		return player->get_pos();
	}	
	return NULL;
}

static void do_patrol(monster_struct *monster)
{
	if (!monster->config)
		return;
	
	// struct position *cur_pos = monster->get_pos();
	
	// if (monster->create_config &&
	// 	(fabsf(cur_pos->pos_x - monster->get_born_pos_x()) > (int)(monster->ai_config->GuardRange)
	// 		|| fabsf(cur_pos->pos_z - monster->get_born_pos_z()) > (int)(monster->ai_config->GuardRange)))
	// {
	// 	return monster->go_back();
	// }
	
	if (monster->is_unit_in_move())
		return;

	if (monster->try_active_attack())
	{
		monster->ai_state = AI_PURSUE_STATE;
		monster->data->target_pos.pos_x = 0;
		monster->data->target_pos.pos_z = 0;
		return;
	}
	
	monster->reset_pos();

	struct position *my_pos = monster->get_pos();
	struct position *his_pos = get_sight_player_pos(monster);
	if (!his_pos)
		return;
	if (get_circle_random_position_v2(monster->scene, my_pos, his_pos, 5, &monster->data->move_path.pos[1]))
	{
		monster->send_patrol_move();		
	}
}

static void ai_tick_17(monster_struct *monster)
{
	if (monster->ai_state == AI_DEAD_STATE)
		return;

	if (monster->is_in_lock_time())
		return;
	if (monster->buff_state & BUFF_STATE_STUN)
		return;

	if (!monster->is_alive())
		assert(0);

	assert(monster->scene);

	switch (monster->ai_state)
	{
		case AI_ATTACK_STATE:
			do_normal_attack(monster);
			break;
		case AI_PURSUE_STATE:  //追击
			do_normal_pursue(monster);
			break;
		case AI_PATROL_STATE:  //巡逻			
			monster->data->ontick_time += monster->count_rand_patrol_time();
		 	do_patrol(monster);
		 	break;
	}
}


static void ai_beattack_17(monster_struct *monster, unit_struct *player)
{
}

struct ai_interface monster_ai_17_interface =
{
	ai_tick_17,
	ai_beattack_17,
	ai_dead_17,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
};




