#include <math.h>
#include <stdlib.h>
#include "game_event.h"
#include "monster_ai.h"
#include "path_algorithm.h"
#include "player_manager.h"
#include "time_helper.h"
#include "unit.h"
#include "buff.h"
#include "check_range.h"
#include "cached_hit_effect.h"
#include "count_skill_damage.h"
#include "msgid.h"
static bool get_22_ai_next_pos(monster_struct * monster, float *pos_x, float *pos_z)
{
	assert(monster->create_config->n_TargetInfoList > 0);
	if (monster->ai_data.circle_ai.cur_pos_index < 0 || monster->ai_data.circle_ai.cur_pos_index >= monster->create_config->n_TargetInfoList)
	{
		return false;
	}
	*pos_x = monster->create_config->TargetInfoList[monster->ai_data.circle_ai.cur_pos_index]->TargetPos->TargetPosX;
	*pos_z = monster->create_config->TargetInfoList[monster->ai_data.circle_ai.cur_pos_index]->TargetPos->TargetPosZ;
	++monster->ai_data.circle_ai.cur_pos_index;
	
	if (monster->ai_data.circle_ai.cur_pos_index >= monster->create_config->n_TargetInfoList)
	{
		monster->ai_data.circle_ai.cur_pos_index = monster->create_config->n_TargetInfoList-1;
	}
	return true;
}

static void do_type22_ai_wait(monster_struct *monster)
{
	monster->ai_state = AI_PATROL_STATE;
	float pos_x, pos_z;
	if(!get_22_ai_next_pos(monster, &pos_x, &pos_z))
		return;
	monster->reset_pos();
	monster->data->move_path.pos[1].pos_x = pos_x;
	monster->data->move_path.pos[1].pos_z = pos_z;
	monster->send_patrol_move();
	return;
}



static void ai_tick_22(monster_struct *monster)
{
	if (monster->ai_type != 22)
		return;
	
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
		case AI_WAIT_STATE:
			do_type22_ai_wait(monster);
			break;
		case AI_ATTACK_STATE:
			do_normal_attack(monster);
			break;
		case AI_PURSUE_STATE:
			do_normal_pursue(monster);
			break;
		case AI_PATROL_STATE:
			do_circlea_or_type22_ai_patrol(monster);
			break;
	}
}






struct ai_interface monster_ai_22_interface =
{
	ai_tick_22,
	circle_ai_beattack,
	circle_ai_dead,
	circle_ai_befly,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	.on_monster_ai_check_goback = circle_ai_check_goback,
	NULL,
	.on_monster_ai_do_goback = circle_ai_do_goback,	
};




