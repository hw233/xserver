//任务召唤怪物
#include <math.h>
#include <stdlib.h>
#include "game_event.h"
#include "monster_ai.h"
#include "monster_manager.h"
#include "path_algorithm.h"
#include "player_manager.h"
#include "time_helper.h"
#include "camp_judge.h"
#include "unit.h"
#include "check_range.h"
#include "cached_hit_effect.h"
#include "count_skill_damage.h"
#include "msgid.h"

static void ai_dead_5(monster_struct *monster, scene_struct *scene)
{
	monster_manager::delete_monster(monster);
}

static void ai_tick_5(monster_struct *monster)
{
	player_struct *target = (player_struct *)monster->target;
	assert(target && target->is_avaliable());
	
//	uint32_t skill_id = choose_rand_skill(monster);
//	monster_hit_notify_to_many_player(skill_id, monster, &target);
	return;
}

static void ai_onalive_5(monster_struct *monster)
{
	assert(monster->data->cur_sight_player == 1);
	player_struct *player = player_manager::get_player_by_id(monster->data->sight_player[0]);
	assert(player && player->is_avaliable());
	monster->target = player;
}

static bool ai_player_leave_sight_5(monster_struct *monster, player_struct *player)
{
	ai_dead_5(monster, monster->scene);
	return false;
}

struct ai_interface monster_ai_5_interface =
{
	ai_tick_5,
	NULL,
	ai_dead_5,
	NULL,
	ai_onalive_5,
	ai_player_leave_sight_5,
};

