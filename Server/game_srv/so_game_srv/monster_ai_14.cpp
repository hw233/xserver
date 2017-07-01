//护送怪物AI
#include <math.h>
#include <stdlib.h>
#include "game_event.h"
#include "monster_ai.h"
#include "path_algorithm.h"
#include "player_manager.h"
#include "time_helper.h"
#include "unit.h"
#include "check_range.h"
#include "cached_hit_effect.h"
#include "count_skill_damage.h"
#include "msgid.h"
#include "monster_manager.h"

static int get_escort_ai_wait_time(monster_struct * monster)
{
	assert(monster->create_config->n_TargetInfoList > 0);
	assert(monster->ai_data.escort_ai.cur_pos_index < monster->create_config->n_TargetInfoList);
	return monster->create_config->TargetInfoList[monster->ai_data.escort_ai.cur_pos_index]->RemainTime;
}

static bool get_escort_ai_next_pos(monster_struct * monster, float *pos_x, float *pos_z)
{
	assert(monster->create_config->n_TargetInfoList > 0);
	++monster->ai_data.escort_ai.cur_pos_index;
	if (monster->ai_data.escort_ai.cur_pos_index >= monster->create_config->n_TargetInfoList)
	{
		return false;
	}
	*pos_x = monster->create_config->TargetInfoList[monster->ai_data.escort_ai.cur_pos_index]->TargetPos->TargetPosX;
	*pos_z = monster->create_config->TargetInfoList[monster->ai_data.escort_ai.cur_pos_index]->TargetPos->TargetPosZ;
	return (true);
}

static bool get_escort_ai_cur_pos(monster_struct * monster, float *pos_x, float *pos_z)
{
	assert(monster->create_config->n_TargetInfoList > 0);
	if (monster->ai_data.escort_ai.cur_pos_index >= monster->create_config->n_TargetInfoList)
	{
		return false;
	}
	*pos_x = monster->create_config->TargetInfoList[monster->ai_data.escort_ai.cur_pos_index]->TargetPos->TargetPosX;
	*pos_z = monster->create_config->TargetInfoList[monster->ai_data.escort_ai.cur_pos_index]->TargetPos->TargetPosZ;
	return (true);
}

static void do_attack(monster_struct *monster)
{
	
}

static void do_wait(monster_struct *monster)
{
	float pos_x, pos_z;
	if (!get_escort_ai_next_pos(monster, &pos_x, &pos_z))
	{
			//到达目的地了
		player_struct *player = player_manager::get_player_by_id(monster->ai_data.escort_ai.owner_player_id);
		if (player)
			player->stop_escort(monster->ai_data.escort_ai.escort_id, true);
		return;
	}
	monster->reset_pos();
	monster->data->move_path.pos[1].pos_x = pos_x;
	monster->data->move_path.pos[1].pos_z = pos_z;
	monster->send_patrol_move();
	monster->ai_state = AI_PATROL_STATE;
}

static void do_patrol(monster_struct *monster)
{
	if (!monster->config)
		return;

	if (monster->is_unit_in_move())
		return;

	monster->ai_state = AI_WAIT_STATE;
	monster->data->ontick_time = time_helper::get_cached_time() + get_escort_ai_wait_time(monster) * 1000;
	return;
}

static void	do_pursue(monster_struct *monster)
{
	if (!monster->data)
		return;
	if (monster->ai_type != AI_TYPE_ESCORT)
		return;

		//脱战巡逻
	if (time_helper::get_cached_time() >= monster->ai_data.escort_ai.out_fight_time)
	{
		monster->ai_state = AI_PATROL_STATE;
		monster->ai_data.escort_ai.out_fight_time = 0;

		float pos_x, pos_z;
		if (!get_escort_ai_cur_pos(monster, &pos_x, &pos_z))
		{
				//到达目的地了
			player_struct *player = player_manager::get_player_by_id(monster->ai_data.escort_ai.owner_player_id);
			if (player)
				player->stop_escort(monster->ai_data.escort_ai.escort_id, true);
			return;
		}
		monster->reset_pos();
		monster->data->move_path.pos[1].pos_x = pos_x;
		monster->data->move_path.pos[1].pos_z = pos_z;
		monster->send_patrol_move();
		monster->ai_state = AI_PATROL_STATE;
		return;
	}

	if (!monster->ai_data.escort_ai.fight_back)
	{
		return;
	}

	monster->on_pursue();
			
	if (!monster->target || !monster->target->is_avaliable()
		|| !monster->target->is_alive())
	{
		return;
	}

	struct position *my_pos = monster->get_pos();
	struct position *his_pos = monster->target->get_pos();

	if (check_distance_in_range(my_pos, his_pos, monster->ai_config->ActiveAttackRange))
	{
		monster->reset_pos();
			// 释放技能攻击目标
		if (monster->config->n_Skill > 0)
		{
			monster_cast_immediate_skill_to_player(monster->config->Skill[0], monster, NULL, monster->target);
		}
		
	}
}

static void monster_ai_14_tick(monster_struct *monster)
{
	if (monster->ai_type != AI_TYPE_ESCORT)
		return;
	
	if (monster->ai_state == AI_DEAD_STATE)
		return;

	if (monster->is_in_lock_time())
		return;
	
	assert(monster->scene);
	area_struct *area = monster->area;
	assert(area);
//	if (!area->is_all_neighbour_have_player())
//		return;
	switch (monster->ai_state)
	{
		case AI_WAIT_STATE:
			do_wait(monster);
			break;
		case AI_ATTACK_STATE:
			do_attack(monster);
			break;
		case AI_PURSUE_STATE:
			do_pursue(monster);
			break;
		case AI_PATROL_STATE:
			do_patrol(monster);
			break;
		default:
			break;
	}
}

static void monster_ai_14_beattack(monster_struct *monster, unit_struct *player)
{
	EscortTask *config = get_config_by_id(monster->ai_data.escort_ai.escort_id, &escort_config);
	if (!config)
	{
		return;
	}

	if (config->BlockedStop == 2)
	{
		return;
	}
	
	monster->ai_state = AI_PURSUE_STATE;
	monster->data->target_pos.pos_x = 0;
	monster->data->target_pos.pos_z = 0;	
	monster->ai_data.escort_ai.out_fight_time = time_helper::get_cached_time() + 5000;
	if (config->BlockedStop == 1)
	{
		monster->ai_data.escort_ai.fight_back = false;
	}
	else if (config->BlockedStop == 3)
	{
		monster->ai_data.escort_ai.fight_back = true;
	}
	if (monster->target && monster->target->is_avaliable())
		return;
	monster->target = player;
}

static void monster_ai_14_befly(monster_struct *monster, unit_struct *player)
{
}

static void monster_ai_14_dead(monster_struct *monster, scene_struct *scene)
{
	monster->ai_state = AI_DEAD_STATE;
	player_struct *player = player_manager::get_player_by_id(monster->ai_data.escort_ai.owner_player_id);
	if (player)
		player->stop_escort(monster->ai_data.escort_ai.escort_id, false);	
}

struct ai_interface monster_ai_14_interface =
{
	monster_ai_14_tick,
	monster_ai_14_beattack,
	monster_ai_14_dead,
	monster_ai_14_befly,
	NULL,
	NULL
};




