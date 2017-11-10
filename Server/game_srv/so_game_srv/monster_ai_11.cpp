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
#include "raid.h"

#define DEAD_BUFF_ID1 114400014
#define DEAD_BUFF_ID2 114400015

static void ai_dead_11(monster_struct *monster, scene_struct *scene)
{
	monster->ai_state = AI_DEAD_STATE;
//	buff_manager::create_default_buff(DEAD_BUFF_ID1, monster, monster, true);
//	buff_manager::create_default_buff(DEAD_BUFF_ID2, monster, monster, true);	
}

static void ai_hp_changed_11(monster_struct *monster)
{
	if (monster->is_alive())
		return;
	monster->set_attr(PLAYER_ATTR_HP, 1);
	buff_manager::create_default_buff(DEAD_BUFF_ID1, monster, monster, true);
	buff_manager::create_default_buff(DEAD_BUFF_ID2, monster, monster, true);		
	if(monster != NULL && monster->data != NULL && monster->scene != NULL && monster->scene->get_scene_type() == SCENE_TYPE_RAID)
	{
		raid_struct *raid = (raid_struct *)monster->scene;
		if(raid == NULL)
			return;
		bool flag = false;
		for (std::set<monster_struct *>::iterator ite = raid->m_monster.begin(); ite != raid->m_monster.end(); ite++)
		{
			monster_struct *p = *ite;
			if(p->data->player_id == monster->data->player_id)
				flag = true;
		}
		if(flag == true)
			return;
		raid->m_monster.insert(monster);
	}
	
//	monster->data->attrData[PLAYER_ATTR_HP] = monster->data->attrData[PLAYER_ATTR_MAXHP];
}

extern void normal_ai_tick(monster_struct *monster);
static void ai_tick_11(monster_struct *monster)
{
//	uint64_t now = time_helper::get_cached_time();

	normal_ai_tick(monster);
}


static void ai_beattack_11(monster_struct *monster, unit_struct *player)
{
}

struct ai_interface monster_ai_11_interface =
{
	ai_tick_11,
	ai_beattack_11,
	ai_dead_11,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	.on_hp_changed = ai_hp_changed_11,
};




