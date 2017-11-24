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

static void choose_target(monster_struct *monster, player_struct *owner, double distance, std::vector<unit_struct *> *ret)
{
	double d = distance * distance;
	struct position *pos = monster->get_pos();
		
	for (int i = 0; i < monster->data->cur_sight_player; ++i)
	{
		player_struct *t_player = player_manager::get_player_by_id(monster->data->sight_player[i]);
//		if (!check_can_attack(monster, t_player))
		if (get_unit_fight_type(owner, t_player) != UNIT_FIGHT_TYPE_ENEMY)			
			continue;
		
		struct position *t_pos = t_player->get_pos();
		double x = (t_pos->pos_x - pos->pos_x);
		double z = (t_pos->pos_z - pos->pos_z);
		double t = x * x + z * z;
		if (t <= d)
		{
			ret->push_back(t_player);
		}
	}
	for (int i = 0; i < monster->data->cur_sight_monster; ++i)
	{
		monster_struct *t_player = monster_manager::get_monster_by_id(monster->data->sight_monster[i]);
//		if (!check_can_attack(monster, t_player))
		if (get_unit_fight_type(owner, t_player) != UNIT_FIGHT_TYPE_ENEMY)					
			continue;
		
		struct position *t_pos = t_player->get_pos();
		double x = (t_pos->pos_x - pos->pos_x);
		double z = (t_pos->pos_z - pos->pos_z);
		double t = x * x + z * z;

//		LOG_DEBUG("%s: %lu t_pos[%.2f][%.2f] t[%.2f] d[%.2f]", __FUNCTION__, monster->data->sight_monster[i], t_pos->pos_x, t_pos->pos_z, t, d);
		
		if (t <= d)
		{
			ret->push_back(t_player);			
		}
	}
}

static void ai_dead_4(monster_struct *monster, scene_struct *scene)
{
	if (monster->scene)
		monster->scene->delete_monster_from_scene(monster, true);
	monster_manager::delete_monster(monster);
}

static void ai_tick_4(monster_struct *monster)
{
//	LOG_DEBUG("%s %p", __FUNCTION__, monster);
		// 检查生命期
	if (check_monster_relive(monster))
	{
		ai_dead_4(monster, monster->scene);
		return;
	}

	player_struct *owner = player_manager::get_player_by_id(monster->data->owner);
	if (!owner)
		return;

		//寻找目标攻击目标
	std::vector<unit_struct *> target;
	choose_target(monster, owner, monster->ai_config->ActiveAttackRange, &target);
	if (target.empty())
		return;
	
	uint32_t skill_id = choose_first_skill(monster);
	if (skill_id == 0)
		return;
	hit_notify_to_many_target(skill_id, monster, &target);

	LOG_DEBUG("%s: %p", __FUNCTION__, monster);
	ai_dead_4(monster, monster->scene);
	return;
	
}

static void ai_beattack_4(monster_struct *monster, unit_struct *player)
{
}

static void ai_befly_4(monster_struct *monster, unit_struct *player)
{
}

static void ai_alive_4(monster_struct *monster)
{
		// TODO: 添加无敌buff
	monster->data->relive_time = monster->ai_config->Regeneration * 1000 + time_helper::get_cached_time();
}

struct ai_interface monster_ai_4_interface =
{
	ai_tick_4,
	ai_beattack_4,
	ai_dead_4,
	ai_befly_4,
	ai_alive_4,
	NULL,
};

