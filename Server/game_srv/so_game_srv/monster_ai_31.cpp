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

static void ai_dead_31(monster_struct *monster, scene_struct *scene)
{

	if(scene->get_scene_type() != SCENE_TYPE_RAID)
		return;
	std::map<uint64_t, struct MGLYshoulingTable*>::iterator itr = maogui_shouling_to_xiaoguai_config.find(monster->data->monster_id); 
	if(itr == maogui_shouling_to_xiaoguai_config.end())
		return;
	raid_struct *raid = (raid_struct *)scene;
	for (std::set<monster_struct *>::iterator ite = raid->m_monster.begin(); ite != raid->m_monster.end();)
	{
		std::set<monster_struct *>::iterator next_ite = ite;
		++next_ite;
		monster_struct *p = *ite;
		for(size_t i = 0; i < itr->second->n_MonsterID; i++)
		{
			if (p->data->monster_id == itr->second->MonsterID[i])
			{
				p->broadcast_one_attr_changed(PLAYER_ATTR_HP, 0, false, false);
				raid->delete_monster_from_scene(p, false);
				monster_manager::delete_monster(p);
				break;
			}
		}
		ite = next_ite;
	}
}

extern void normal_ai_tick(monster_struct *monster);
static void ai_tick_31(monster_struct *monster)
{
//	uint64_t now = time_helper::get_cached_time();

	normal_ai_tick(monster);
}


static void ai_beattack_31(monster_struct *monster, unit_struct *player)
{
}

struct ai_interface monster_ai_31_interface =
{
	ai_tick_31,
	ai_beattack_31,
	ai_dead_31,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
};




