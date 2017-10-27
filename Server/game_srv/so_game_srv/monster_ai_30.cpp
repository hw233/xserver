#include <math.h>
#include <stdlib.h>
#include "game_event.h"
#include "monster_ai.h"
#include "path_algorithm.h"
#include "player_manager.h"
#include "time_helper.h"
#include "unit.h"
#include "buff.h"
#include "raid.h"
#include "check_range.h"
#include "cached_hit_effect.h"
#include "count_skill_damage.h"
#include "msgid.h"
bool get_30_ai_next_pos(monster_struct * monster, float *pos_x, float *pos_z)
{
	assert(monster->create_config->n_TargetInfoList > 0);
	if (monster->ai_data.circle_ai.cur_pos_index < 0 || monster->ai_data.circle_ai.cur_pos_index >= monster->create_config->n_TargetInfoList)
	{
		return false;
	}
	*pos_x = monster->create_config->TargetInfoList[monster->ai_data.circle_ai.cur_pos_index]->TargetPos->TargetPosX;
	*pos_z = monster->create_config->TargetInfoList[monster->ai_data.circle_ai.cur_pos_index]->TargetPos->TargetPosZ;
	++monster->ai_data.circle_ai.cur_pos_index;
	
	
	return true;
}

void do_type30_ai_wait(monster_struct *monster)
{
	monster->ai_state = AI_PATROL_STATE;
	float pos_x, pos_z;
	if (!get_30_ai_next_pos(monster, &pos_x, &pos_z))
	{
		if (monster->scene->get_scene_type() == SCENE_TYPE_RAID)
		{
			raid_struct *raid = (raid_struct *)monster->scene;
			if (raid->ai != NULL && raid->ai->raid_on_escort_end_piont != NULL)
			{
				raid->ai->raid_on_escort_end_piont(raid, monster);
			}
		}
		return;
	}	
	monster->reset_pos();
	monster->data->move_path.pos[1].pos_x = pos_x;
	monster->data->move_path.pos[1].pos_z = pos_z;
	monster->send_patrol_move();
	return;
}

static void ai_tick_30(monster_struct *monster)
{
	if (monster->ai_type != 30)
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
			do_type30_ai_wait(monster);
			break;
		case AI_PATROL_STATE:
			do_type30_ai_patrol(monster);
			break;
	}
}






struct ai_interface monster_ai_30_interface =
{
	ai_tick_30,
	NULL,
	normal_ai_dead,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
};




