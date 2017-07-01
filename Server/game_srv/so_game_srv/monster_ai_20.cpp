#include <math.h>
#include <stdlib.h>
#include "game_event.h"
#include "monster_ai.h"
#include "monster_manager.h"
#include "path_algorithm.h"
#include "player_manager.h"
#include "time_helper.h"
#include "unit.h"
#include "attr_calc.h"
#include "collect.h"
#include "check_range.h"
#include "cached_hit_effect.h"
#include "camp_judge.h"
#include "count_skill_damage.h"
#include "msgid.h"
#include "buff.h"
#include "buff_manager.h"

//151002012巅峰谷500年前-妖帝人形-写死ID（远程）
//151002013巅峰谷500年前-妖帝真身-写死ID（近战）
#define NEW_YAODI_ID 151002013

static void ai_dead_20(monster_struct *monster, scene_struct *scene)
{
	monster->ai_state = AI_DEAD_STATE;
}

extern void normal_ai_tick(monster_struct *monster);
static void ai_tick_20(monster_struct *monster)
{
//	uint64_t now = time_helper::get_cached_time();

		//要在这里处理变身
	if (monster->ai_data.yaodi_ai.state == 1)
	{
		monster->config = get_config_by_id(NEW_YAODI_ID, &monster_config);
		monster->data->monster_id = NEW_YAODI_ID;
		::get_attr_from_config(monster->config->BaseAttribute * 1000 + monster->get_attr(PLAYER_ATTR_LEVEL), monster->data->attrData, &monster->drop_id);
		monster->ai_data.yaodi_ai.state = 2;
		if (monster->config->n_Skill >= 3)
			monster->data->next_skill_id = monster->config->Skill[2];
		// 发送变身协议
		SightMonsterIdChangeNotify nty;
		sight_monster_id_change_notify__init(&nty);
		nty.uuid = monster->get_uuid();
		nty.monster_id = NEW_YAODI_ID;
		monster->broadcast_to_sight(MSG_ID_MONSTER_ID_CHANGED_NOTIFY, &nty, (pack_func)sight_monster_id_change_notify__pack, false);
		
		// struct position *pos = monster->get_pos();
		// int lv = monster->get_attr(PLAYER_ATTR_LEVEL);
		// monster_struct *new_monster = monster_manager::create_monster_at_pos(monster->scene, NEW_YAODI_ID, lv, pos->pos_x, pos->pos_z);
		// if (!new_monster)
		// 	return;
		// int hp = monster->get_attr(PLAYER_ATTR_HP);
		// new_monster->set_attr(PLAYER_ATTR_HP, hp);
		// new_monster->broadcast_one_attr_changed(PLAYER_ATTR_HP, hp, false, false);
				
		// monster->scene->delete_monster_from_scene(monster, true);
		// monster_manager::delete_monster(monster);
				
		// new_monster->ai_data.yaodi_ai.state = 2;
		// if (new_monster->config->n_Skill >= 3)
		// 	new_monster->data->next_skill_id = monster->config->Skill[2];


		
		return;
	}
	
	normal_ai_tick(monster);
}


static void ai_beattack_20(monster_struct *monster, unit_struct *player)
{
	int percent = get_monster_hp_percent(monster);
	switch (monster->ai_data.yaodi_ai.state)
	{
		case 0:
			if (percent <= 50)
			{
				monster->ai_data.yaodi_ai.state = 1;				
			}
			break;
		case 2:
			if (percent <= 20)
			{
				monster->ai_data.yaodi_ai.state = 3;
				if (monster->config->n_Skill >= 3)
					monster->data->next_skill_id = monster->config->Skill[2];
			}
			break;
		default:
			break;
	}
}

static void ai_monster_init_20(monster_struct *monster, unit_struct *owner)
{
}

struct ai_interface monster_ai_20_interface =
{
	.on_tick = ai_tick_20,
	.on_beattack = ai_beattack_20,
	ai_dead_20,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	.on_monster_ai_init = ai_monster_init_20,
};




