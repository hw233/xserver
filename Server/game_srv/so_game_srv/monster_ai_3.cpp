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

void monster_ai_3_tick(monster_struct *monster)
{
	if (monster->ai_state == AI_DEAD_STATE)
		return do_normal_dead(monster);
}

void monster_ai_3_dead(monster_struct *monster, scene_struct *scene)
{
	monster->ai_state = AI_DEAD_STATE;
}

struct ai_interface monster_ai_3_interface =
{
	.on_tick = monster_ai_3_tick,
	NULL,
	.on_dead = monster_ai_3_dead,
};
