//蒋雳夫等4人AI
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

#define STATE1_BUFF_ID1 114400016
#define STATE1_BUFF_ID2 114400017
#define STATE2_BUFF_ID1 114400014
#define STATE2_BUFF_ID2 114400015

static void ai_dead_21(monster_struct *monster, scene_struct *scene)
{
	monster->ai_state = AI_DEAD_STATE;
}

extern void normal_ai_tick(monster_struct *monster);
static void ai_tick_21(monster_struct *monster)
{
//	uint64_t now = time_helper::get_cached_time();
	if (monster->ai_data.type21_ai.state == 2)
		return;
	
	normal_ai_tick(monster);
}


static void ai_beattack_21(monster_struct *monster, unit_struct *player)
{
	switch (monster->ai_data.type21_ai.state)
	{
		case 0:
		{
			int percent = get_monster_hp_percent(monster);
			if (percent <= 50)
			{
				monster->ai_data.type21_ai.state = 1;
				buff_manager::create_buff(STATE1_BUFF_ID1, monster, monster, true);
				buff_manager::create_buff(STATE1_BUFF_ID2, monster, monster, true);	
			}
		}
		break;
		case 1:
		{
			int percent = get_monster_hp_percent(monster);
			if (percent <= 10)
			{
				monster->ai_data.type21_ai.state = 2;
				buff_manager::create_buff(STATE2_BUFF_ID1, monster, monster, true);
				buff_manager::create_buff(STATE2_BUFF_ID2, monster, monster, true);	
			}
		}
		break;
		default:
		{
		}
		break;
	}
}

struct ai_interface monster_ai_21_interface =
{
	ai_tick_21,
	ai_beattack_21,
	ai_dead_21,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
};




