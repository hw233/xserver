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

static void ai_dead_15(monster_struct *monster, scene_struct *scene)
{
	monster->ai_state = AI_DEAD_STATE;
}

extern void normal_ai_tick(monster_struct *monster);
static void ai_tick_15(monster_struct *monster)
{
//	uint64_t now = time_helper::get_cached_time();

	normal_ai_tick(monster);
}


static void ai_beattack_15(monster_struct *monster, unit_struct *player)
{
}

static void ai_attack_player_15(monster_struct *monster, player_struct *player, int damage)
{
	if (damage <= 0)
		return;
	boss_struct *boss = monster_manager::get_boss_by_id(monster->ai_data.add_boss_hp_ai.boss_uuid);
	if (!boss)
		return;
	double *attr = boss->get_all_attr();	
	assert(attr);
	if (attr[PLAYER_ATTR_HP] == attr[PLAYER_ATTR_MAXHP])
		return;
	
	attr[PLAYER_ATTR_HP] += damage;
	if (attr[PLAYER_ATTR_HP] > attr[PLAYER_ATTR_MAXHP])
		attr[PLAYER_ATTR_HP] = attr[PLAYER_ATTR_MAXHP];

	boss->on_hp_changed(-damage);
	boss->broadcast_one_attr_changed(PLAYER_ATTR_HP, attr[PLAYER_ATTR_HP], false, false);		
}

static void ai_monster_init_15(monster_struct *monster, unit_struct *owner)
{
	if (!owner)
		return;
	monster->ai_data.add_boss_hp_ai.boss_uuid = owner->get_uuid();
}

struct ai_interface monster_ai_15_interface =
{
	ai_tick_15,
	ai_beattack_15,
	ai_dead_15,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	ai_attack_player_15,
	NULL,
	.on_monster_ai_init = ai_monster_init_15,
};




