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

extern void normal_ai_tick(monster_struct *monster);
static void ai_tick_19(monster_struct *monster)
{
//	uint64_t now = time_helper::get_cached_time();

	normal_ai_tick(monster);
}


static void ai_beattack_19(monster_struct *monster, unit_struct *player)
{
}

static void ai_monster_init_19(monster_struct *monster, unit_struct *owner)
{
	if (monster->config->n_Skill >= 2)
		monster->data->next_skill_id = monster->config->Skill[1];
}

struct ai_interface monster_ai_19_interface =
{
	ai_tick_19,
	ai_beattack_19,
	normal_ai_dead,
	NULL,
	NULL,
	NULL,	
	NULL,
	NULL,
	NULL,
	NULL,
	.on_monster_ai_init = ai_monster_init_19,
};




