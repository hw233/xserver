#include <math.h>
#include <stdlib.h>
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

static void	do_patrol(monster_struct *monster, uint16_t last_ai_state)
{
	if (!monster->config)
		return;

	if (last_ai_state != AI_PATROL_STATE)
		return monster->go_back();

	if (monster->is_unit_in_move())
		return;

	if (monster->try_active_attack())
	{
		monster->ai_state = AI_PURSUE_STATE;
		struct position *target_pos = monster->target->get_pos();
		monster->data->target_pos.pos_x = target_pos->pos_x;
		monster->data->target_pos.pos_z = target_pos->pos_z;
		return;
	}
}

static void monster_ai_26_tick(monster_struct *monster)
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
	uint16_t last_ai_state = monster->ai_data.type26_ai.last_ai_state;
	monster->ai_data.type26_ai.last_ai_state = monster->ai_state;
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
		do_patrol(monster, last_ai_state);
		break;
	}
}

//巡逻中被攻击则进入追击
static void monster_ai_26_beattack(monster_struct *monster, unit_struct *player)
{
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
}

//被击飞击退击倒
static void monster_ai_26_befly(monster_struct *monster, unit_struct *player)
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

static bool	monster_ai_26_player_leave_sight(monster_struct *monster, player_struct *player)
{
	if (monster->target && monster->target->get_uuid() == player->data->player_id)
		monster->target = NULL;
	return true;
}

struct ai_interface monster_ai_26_interface =
{
	monster_ai_26_tick,
	monster_ai_26_beattack,
	normal_ai_dead,
	monster_ai_26_befly,
	NULL,
	monster_ai_26_player_leave_sight,
};




