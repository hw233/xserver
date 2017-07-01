#include <math.h>
#include <stdlib.h>
#include "game_event.h"
#include "monster_ai.h"
#include "monster_manager.h"
#include "path_algorithm.h"
#include "player_manager.h"
#include "time_helper.h"
#include "unit.h"
#include "check_range.h"
#include "cached_hit_effect.h"
#include "camp_judge.h"
#include "count_skill_damage.h"
#include "msgid.h"

static unit_struct *choose_target(monster_struct *monster, player_struct *owner, double distance)
{
	unit_struct *ret = NULL;
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
		if (t < d)
		{
			ret = t_player;
			d = t;
		}
	}
	for (int i = 0; i < monster->data->cur_sight_monster; ++i)
	{
		monster_struct *t_player = monster_manager::get_monster_by_id(monster->data->sight_monster[i]);
		if (!t_player)
		{
			LOG_ERR("%s: %lu can not find sight monster %lu", __FUNCTION__, monster->get_uuid(), monster->data->sight_monster[i]);
			continue;
		}
		
//		if (!check_can_attack(monster, t_player))
		if (get_unit_fight_type(owner, t_player) != UNIT_FIGHT_TYPE_ENEMY)							
			continue;
		
		struct position *t_pos = t_player->get_pos();
		double x = (t_pos->pos_x - pos->pos_x);
		double z = (t_pos->pos_z - pos->pos_z);
		double t = x * x + z * z;
		if (t < d)
		{
			ret = t_player;
			d = t;
		}
	}
//	if (d > distance * distance)
//		return NULL;
	return ret;
}

static void ai_dead_0(monster_struct *monster, scene_struct *scene)
{
	if (monster->scene)
		monster->scene->delete_monster_from_scene(monster, true);
	monster_manager::delete_monster(monster);
}

static void ai_tick_0(monster_struct *monster)
{
//	LOG_DEBUG("%s %p", __FUNCTION__, monster);
		// 检查生命期
	if (check_monster_relive(monster))
	{
		ai_dead_0(monster, monster->scene);
		return;
	}

	player_struct *owner = player_manager::get_player_by_id(monster->data->owner);
	if (!owner)
		return;

		//寻找目标攻击目标
	unit_struct *target = choose_target(monster, owner, monster->ai_config->ActiveAttackRange);
	if (!target)
		return;
	
	uint32_t skill_id = choose_first_skill(monster);
	if (skill_id == 0)
		return;
	struct SkillTable *config = get_config_by_id(skill_id, &skill_config);
	if (!config)
		return;
	monster_cast_immediate_skill_to_player(skill_id, monster, owner, target);
		//计算硬直时间
	monster->data->ontick_time += count_skill_delay_time(config);	
}

//巡逻中被攻击则进入追击
static void ai_beattack_0(monster_struct *monster, unit_struct *player)
{
}

//被击飞击退击倒
static void ai_befly_0(monster_struct *monster, unit_struct *player)
{
}

static void ai_alive_0(monster_struct *monster)
{
		// TODO: 添加无敌buff
	monster->data->relive_time = monster->ai_config->Regeneration * 1000 + time_helper::get_cached_time();
}

struct ai_interface monster_ai_0_interface =
{
	ai_tick_0,
	ai_beattack_0,
	ai_dead_0,
	ai_befly_0,
	ai_alive_0,
	NULL
};






