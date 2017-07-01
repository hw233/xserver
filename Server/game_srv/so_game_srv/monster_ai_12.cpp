#include <math.h>
#include <stdlib.h>
#include "game_event.h"
#include "monster_ai.h"
#include "monster_manager.h"
#include "path_algorithm.h"
#include "player_manager.h"
#include "time_helper.h"
#include "unit.h"
#include "raid.h"
#include "collect.h"
#include "check_range.h"
#include "cached_hit_effect.h"
#include "camp_judge.h"
#include "count_skill_damage.h"
#include "msgid.h"
#include "buff.h"
#include "buff_manager.h"

static void ai_dead_12(monster_struct *monster, scene_struct *scene)
{
	monster->ai_state = AI_DEAD_STATE;
		// 杀死副本里面对应的侍神小怪
	uint32_t monster_id = 0;
	for (size_t i = 0; i < ARRAY_SIZE(sg_shishen_shouling_id); ++i)
	{
		if (sg_shishen_shouling_id[i] == monster->data->monster_id)
		{
			monster_id = sg_shishen_xiaoguai_id[i];
			break;
		}
	}
	assert(monster_id != 0);
	assert(scene && scene->get_scene_type() == SCENE_TYPE_RAID);
	raid_struct *raid = (raid_struct *)scene;
	for (std::set<monster_struct *>::iterator ite = raid->m_monster.begin(); ite != raid->m_monster.end();)
	{
		std::set<monster_struct *>::iterator next_ite = ite;
		++next_ite;
		if ((*ite)->data->monster_id == monster_id)
		{
			raid->delete_monster_from_scene((*ite), true);
			monster_manager::delete_monster(*ite);
		}
		ite = next_ite;
	}
}

extern void normal_ai_tick(monster_struct *monster);
static void ai_tick_12(monster_struct *monster)
{
//	uint64_t now = time_helper::get_cached_time();

	normal_ai_tick(monster);
}


static void ai_beattack_12(monster_struct *monster, unit_struct *player)
{
}

struct ai_interface monster_ai_12_interface =
{
	ai_tick_12,
	ai_beattack_12,
	ai_dead_12,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
};




