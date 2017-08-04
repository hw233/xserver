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

void ai_tick_25(monster_struct *monster)
{
	if (!monster->is_alive())
		return;

	if (monster->is_unit_in_move())
		return;

	monster->set_ai_interface(monster->ai_type);
}

struct ai_interface monster_ai_25_interface =
{
	ai_tick_25,
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
	NULL,
	NULL, 
};




