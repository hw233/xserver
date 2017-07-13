#include <math.h>
#include <stdlib.h>
#include "cash_truck_manager.h"
#include "camp_judge.h"
#include "game_event.h"
#include "monster_ai.h"
#include "monster_manager.h"
#include "path_algorithm.h"
#include "player_manager.h"
#include "time_helper.h"
#include "unit.h"
#include "check_range.h"
#include "cached_hit_effect.h"
#include "count_skill_damage.h"
#include "msgid.h"
#include "buff.h"

void ai_tick_23(monster_struct *monster)
{
	if (monster->ai_state == AI_DEAD_STATE)
		return do_normal_dead(monster);

	if (monster->is_in_lock_time())
		return;
	if (monster->buff_state & BUFF_STATE_STUN)
		return;

	if (!monster->is_alive())
		assert(0);

	assert(monster->scene);
	area_struct *area = monster->area;
	if (area)
	{
		if (!area->is_all_neighbour_have_player())
			return;
	}
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
			do_normal_patrol(monster);
			break;
	}
}

//巡逻中被攻击则进入追击
static void ai_beattack_23(monster_struct *monster, unit_struct *player)
{
		//如果当前目标不是玩家，而且攻击者是玩家，那么切换目标到玩家身上
	if (player->get_unit_type() == UNIT_TYPE_PLAYER)
	{
		if (monster->target && monster->target->is_avaliable()
			&& monster->target->is_alive()
			&& monster->target->get_unit_type() != UNIT_TYPE_PLAYER)
		{
			monster->target = player;
		}
	}
	
	if (monster->ai_state != AI_PATROL_STATE)
		return;
		
	monster->ai_state = AI_PURSUE_STATE;
	monster->data->target_pos.pos_x = 0;
	monster->data->target_pos.pos_z = 0;		
	if (monster->target && monster->target->is_avaliable())
		return;
	monster->target = player;

	uint64_t now = time_helper::get_cached_time();			
	monster->reset_timer(now + 500);
//	monster_manager::monster_ontick_reset_timer(monster);
}

//被击飞击退击倒
static void ai_befly_23(monster_struct *monster, unit_struct *player)
{
		//打断技能
	if (monster->ai_state == AI_ATTACK_STATE)
	{
		monster->ai_state = AI_PURSUE_STATE;
		monster->data->target_pos.pos_x = 0;
		monster->data->target_pos.pos_z = 0;			
		monster->data->skill_id = 0;		
	}
}

static void ai_dead_23(monster_struct *monster, scene_struct *scene)
{
	monster->ai_state = AI_DEAD_STATE;
}

static bool	ai_player_leave_sight_23(monster_struct *monster, player_struct *player)
{
	if (monster->target && monster->target->get_uuid() == player->data->player_id)
		monster->target = NULL;
	return true;
}

static unit_struct *ai_choose_target_23(monster_struct *monster)
{
	assert(monster->ai_config);
	if (monster->ai_config->ActiveAttackRange == 0)
		return NULL;
	double range = monster->ai_config->ActiveAttackRange * monster->ai_config->ActiveAttackRange;
	struct position *my_pos = monster->get_pos();

	for (int i = 0; i < monster->data->cur_sight_monster; ++i)
	{
		monster_struct *target_monster = monster_manager::get_monster_by_id(monster->data->sight_monster[i]);
		if (!target_monster)
			continue;

			//护送怪
		if (target_monster->ai_config->AIType != 14)
			continue;

		if (get_unit_fight_type(monster, target_monster) != UNIT_FIGHT_TYPE_ENEMY)									
			continue;
		
		struct position *pos = target_monster->get_pos();
		double x = pos->pos_x - my_pos->pos_x;
		double z = pos->pos_z - my_pos->pos_z;
		if (x * x + z * z > range)
			continue;
		return target_monster;
	}
	
	for (int i = 0; i < monster->data->cur_sight_truck; ++i)
	{
		cash_truck_struct *truck = cash_truck_manager::get_cash_truck_by_id(monster->data->sight_truck[i]);
		if (!truck)
			continue;
		if (get_unit_fight_type(monster, truck) != UNIT_FIGHT_TYPE_ENEMY)							
			continue;
		
		struct position *pos = truck->get_pos();
		double x = pos->pos_x - my_pos->pos_x;
		double z = pos->pos_z - my_pos->pos_z;
		if (x * x + z * z > range)
			continue;
		return truck;
	}
	for (int i = 0; i < monster->data->cur_sight_player; ++i)
	{
		player_struct *player = player_manager::get_player_by_id(monster->data->sight_player[i]);
		if (!player)
			continue;
		if (get_unit_fight_type(monster, player) != UNIT_FIGHT_TYPE_ENEMY)							
			continue;
		
		struct position *pos = player->get_pos();
		double x = pos->pos_x - my_pos->pos_x;
		double z = pos->pos_z - my_pos->pos_z;
		if (x * x + z * z > range)
			continue;
		return player;
	}
	for (int i = 0; i < monster->data->cur_sight_monster; ++i)
	{
		monster_struct *target_monster = monster_manager::get_monster_by_id(monster->data->sight_monster[i]);
		if (!target_monster)
			continue;

		if (get_unit_fight_type(monster, target_monster) != UNIT_FIGHT_TYPE_ENEMY)									
			continue;
		
		struct position *pos = target_monster->get_pos();
		double x = pos->pos_x - my_pos->pos_x;
		double z = pos->pos_z - my_pos->pos_z;
		if (x * x + z * z > range)
			continue;
		return target_monster;
	}
	return NULL;
}

struct ai_interface monster_ai_23_interface =
{
	ai_tick_23,
	ai_beattack_23,
	ai_dead_23,
	ai_befly_23,
	NULL,
	ai_player_leave_sight_23,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	.monster_ai_choose_target = ai_choose_target_23,
};




