//优先攻击怪物的AI
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

static bool find_next_target(monster_struct *monster)
{
	assert(monster->ai_config);
	if (monster->ai_config->ActiveAttackRange == 0)
		return false;
	double range = monster->ai_config->ActiveAttackRange * monster->ai_config->ActiveAttackRange;
	struct position *my_pos = monster->get_pos();

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
		monster->target = target_monster;
		return true;
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
		monster->target = player;
		return true;
	}
	return false;
}

static void do_pursue(monster_struct *monster)
{
	if (!monster->target || !monster->target->is_avaliable()
		|| !monster->target->is_alive())
	{
		monster->ai_state = AI_PATROL_STATE;
		return;
	}
	if (monster->target->get_unit_type() == UNIT_TYPE_PLAYER)
	{
		find_next_target(monster);
	}
	
	return do_normal_pursue(monster);
}

static void do_patrol(monster_struct *monster)
{
	if (!monster->config)
		return;

	struct position *cur_pos = monster->get_pos();

	if (monster->create_config &&
		(fabsf(cur_pos->pos_x - monster->get_born_pos_x()) > (int)(monster->ai_config->GuardRange)
			|| fabsf(cur_pos->pos_z - monster->get_born_pos_z()) > (int)(monster->ai_config->GuardRange)))
	{
		return monster->go_back();
	}

	if (monster->is_unit_in_move())
		return;

	if (find_next_target(monster))
	{
		monster->ai_state = AI_PURSUE_STATE;
		monster->data->target_pos.pos_x = 0;
		monster->data->target_pos.pos_z = 0;
		return;
	}

	monster->reset_pos();
	if (find_rand_position(monster->scene, &monster->data->move_path.pos[0],
			monster->ai_config->GuardRange, &monster->data->move_path.pos[1]) == 0)
	{
		monster->send_patrol_move();
	}
}


static void ai_dead_16(monster_struct *monster, scene_struct *scene)
{
	monster->ai_state = AI_DEAD_STATE;
}

static void ai_tick_16(monster_struct *monster)
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
			do_pursue(monster);
//			do_normal_pursue(monster);
			break;
		case AI_PATROL_STATE:  //巡逻
			monster->data->ontick_time += monster->count_rand_patrol_time();
			do_patrol(monster);
			break;
	}

}


static void ai_beattack_16(monster_struct *monster, unit_struct *player)
{
}

struct ai_interface monster_ai_16_interface =
{
	ai_tick_16,
	ai_beattack_16,
	ai_dead_16,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
};
