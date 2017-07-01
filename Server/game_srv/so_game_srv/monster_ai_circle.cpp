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

static int get_circle_ai_wait_time(monster_struct * monster)
{
	assert(monster->create_config->n_TargetInfoList > 0);
	assert(monster->ai_data.circle_ai.cur_pos_index < monster->create_config->n_TargetInfoList);
	return monster->create_config->TargetInfoList[monster->ai_data.circle_ai.cur_pos_index]->RemainTime;
}

static int get_circle_ai_next_pos(monster_struct * monster, float *pos_x, float *pos_z)
{
	assert(monster->create_config->n_TargetInfoList > 0);
	++monster->ai_data.circle_ai.cur_pos_index;
	if (monster->ai_data.circle_ai.cur_pos_index >= monster->create_config->n_TargetInfoList)
		monster->ai_data.circle_ai.cur_pos_index = 0;
	*pos_x = monster->create_config->TargetInfoList[monster->ai_data.circle_ai.cur_pos_index]->TargetPos->TargetPosX;
	*pos_z = monster->create_config->TargetInfoList[monster->ai_data.circle_ai.cur_pos_index]->TargetPos->TargetPosZ;
	return (0);
}

static void do_wait(monster_struct *monster)
{
	float pos_x, pos_z;
	get_circle_ai_next_pos(monster, &pos_x, &pos_z);
	monster->reset_pos();
	monster->data->move_path.pos[1].pos_x = pos_x;
	monster->data->move_path.pos[1].pos_z = pos_z;
	monster->send_patrol_move();
	monster->ai_state = AI_PATROL_STATE;
}

void do_circlea_or_type22_ai_patrol(monster_struct *monster)
{
	if (!monster->config)
		return;

	if (monster->try_active_attack())
	{
		monster->ai_state = AI_PURSUE_STATE;
		monster->data->target_pos.pos_x = 0;
		monster->data->target_pos.pos_z = 0;
		struct position *pos = monster->get_pos();
		monster->ai_data.circle_ai.ret_pos.pos_x = pos->pos_x;
		monster->ai_data.circle_ai.ret_pos.pos_z = pos->pos_z;			
		return;
	}

	if (monster->is_unit_in_move())
		return;

	monster->ai_state = AI_WAIT_STATE;
	monster->data->ontick_time = time_helper::get_cached_time() + get_circle_ai_wait_time(monster) * 1000;
	return;
}

// static void	do_pursue(monster_struct *monster)
// {
// 	if (!monster->data)
// 		return;
// 	if (monster->ai_type != AI_TYPE_CIRCLE)
// 		return;

// 	monster->on_pursue();
			
// 	if (!monster->target || !monster->target->is_avaliable()
// 		|| !monster->target->is_alive())
// 	{
// 		monster->ai_state = AI_PATROL_STATE;
// 		return;
// 	}

// 	struct position *my_pos = monster->get_pos();
// 	struct position *his_pos = monster->target->get_pos();

// 	if (check_distance_in_range(my_pos, his_pos, monster->ai_config->ActiveAttackRange))
// 	{
// 		monster->reset_pos();
// 			// TODO: 释放技能攻击目标
// 		if (monster->config->n_Skill > 0)
// 		{
// 			monster_cast_immediate_skill_to_player(monster->config->Skill[0], monster, NULL, monster->target);
// 		}
		
// 	}
// 	else
// 	{
// 			//追击
// 		if (monster->is_unit_in_move())
// 			return;

// 		int direct = getdirection(his_pos, my_pos);
// 		if (get_circle_random_position(monster->scene, direct, my_pos, his_pos, monster->ai_config->ActiveAttackRange * 0.8, &monster->data->move_path.pos[1]))
// 		{
// 			monster->send_patrol_move();			
// 		}

// //		LOG_DEBUG("%s %d: player_pos[%f][%f] move direct[%d] from[%f][%f] to[%f][%f]", __FUNCTION__, __LINE__,
// //			his_pos->pos_x, his_pos->pos_z,	direct, my_pos->pos_x, my_pos->pos_z,
// //			monster->data->move_path.pos[1].pos_x, monster->data->move_path.pos[1].pos_z);
		
// //		monster->data->move_path.pos[1].pos_x = his_pos->pos_x;
// //		monster->data->move_path.pos[1].pos_z = his_pos->pos_z;
// //		send_patrol_move(monster);
// 	}
// }

static void	do_dead(monster_struct *monster)
{
	if (check_monster_relive(monster))
	{
		monster->target = NULL;
		monster->ai_state = AI_PATROL_STATE;
		monster->set_pos(monster->create_config->TargetInfoList[0]->TargetPos->TargetPosX,
			monster->create_config->TargetInfoList[0]->TargetPos->TargetPosZ);
		monster->data->attrData[PLAYER_ATTR_HP] = monster->data->attrData[PLAYER_ATTR_MAXHP];
		monster->on_relive();
	}
}

static void	do_goback(monster_struct *monster)
{
	monster->on_go_back();	
	monster->reset_pos();
	monster->data->move_path.pos[1].pos_x = monster->ai_data.circle_ai.ret_pos.pos_x;
	monster->data->move_path.pos[1].pos_z = monster->ai_data.circle_ai.ret_pos.pos_z;
	monster->send_patrol_move();
}

static void circle_ai_tick(monster_struct *monster)
{
	if (monster->ai_type != AI_TYPE_CIRCLE)
		return;
	
	if (monster->ai_state == AI_DEAD_STATE)
		return do_dead(monster);

	if (monster->is_in_lock_time())
		return;
	if (monster->buff_state & BUFF_STATE_STUN)
		return;
	if (!monster->is_alive())
		assert(0);
	
	assert(monster->scene);
//	area_struct *area = monster->area;
//	assert(area);
//	if (!area->is_all_neighbour_have_player())
//		return;
	switch (monster->ai_state)
	{
		case AI_WAIT_STATE:
			do_wait(monster);
			break;
		case AI_ATTACK_STATE:
			do_normal_attack(monster);
			break;
		case AI_PURSUE_STATE:
			do_normal_pursue(monster);
			break;
//		case AI_GO_BACK_STATE:
//			monster->data->ontick_time += random() % 2000;
//			do_goback(monster);
//			break;
		case AI_PATROL_STATE:
			do_circlea_or_type22_ai_patrol(monster);
			break;
//		case AI_DEAD_STATE:
//			do_dead(monster);
	}
}

void circle_ai_beattack(monster_struct *monster, unit_struct *player)
{
	monster->ai_state = AI_PURSUE_STATE;
	monster->data->target_pos.pos_x = 0;
	monster->data->target_pos.pos_z = 0;		
	if (monster->target && monster->target->is_avaliable())
		return;
	monster->target = player;
	struct position *pos = monster->get_pos();
	monster->ai_data.circle_ai.ret_pos.pos_x = pos->pos_x;
	monster->ai_data.circle_ai.ret_pos.pos_z = pos->pos_z;	
}

void circle_ai_befly(monster_struct *monster, unit_struct *player)
{
}

void circle_ai_dead(monster_struct *monster, scene_struct *scene)
{
	monster->ai_state = AI_DEAD_STATE;
}

bool circle_ai_check_goback(monster_struct *monster)
{
	struct position *my_pos = monster->get_pos();
//	monster->data->move_path.pos[1].pos_x = monster->ai_data.circle_ai.ret_pos.pos_x;
//	monster->data->move_path.pos[1].pos_z = monster->ai_data.circle_ai.ret_pos.pos_z;

	if (fabsf(my_pos->pos_x - monster->ai_data.circle_ai.ret_pos.pos_x) > (int)(monster->ai_config->ChaseRange)
		|| fabsf(my_pos->pos_z - monster->ai_data.circle_ai.ret_pos.pos_z) > (int)(monster->ai_config->ChaseRange))
	{
		do_goback(monster);
		return true;
	}
	
	return false;
}

struct ai_interface monster_ai_circle_interface =
{
	circle_ai_tick,
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
};




