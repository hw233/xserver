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
#include "lua_config.h"
#include "count_skill_damage.h"
#include "game_config.h"
#include "unit_path.h"



//怪物技能释放类型是矩形
// static void skill_rect_type(std::vector<unit_struct*> *target, monster_struct* monster, uint64_t skill_id, double  length, double  width)
// {
// 	if (NULL == monster || monster->create_config == NULL)
// 	{
// 		LOG_DEBUG("%s Monster Or monster->create_config Is Null", __FUNCTION__);
// 		return;
// 	}
// 	struct position *monster_pos = monster->get_pos();

// 	double angle = monster->create_config->Yaw;
// 	double cos = qFastCos(angle);
// 	double sin = qFastSin(angle);


// 	double x1, x2;
// 	double z1, z2;

// 	x1 = 0;
// 	x2 = x1 + length;
// 	z1 = -width;
// 	z2 = width;


// 	for (int i = 0; i < monster->data->cur_sight_player; ++i)
// 	{
		
// 		player_struct *t_player = player_manager::get_player_by_id(monster->data->sight_player[i]);
// 		//monster_cast_skill_to_player(skill_id, monster, t_player);
// 		if (!t_player || !t_player->is_alive())
// 		{
// 			LOG_ERR("%s %d: player[%lu] in sight", __FUNCTION__, __LINE__, monster->data->sight_player[i]);
// 			continue;
// 		}
// 		struct position * target_pos = t_player->get_pos();

// 		double pos_x = target_pos->pos_x - monster_pos->pos_x;
// 		double pos_z = target_pos->pos_z - monster_pos->pos_z;
// 		double target_x1 = cos*(pos_x)-sin*(pos_z);
// 		double target_z1 = cos*(pos_z)+sin*(pos_x);

// 		if (target_x1 >= x1 && target_x1 <= x2 && target_z1 >= z1 && target_z1 <= z2)
// 		{
// 			target->push_back(t_player);
// 		}
// 	}

// 	for (int i = 0; i < monster->data->cur_sight_monster; ++i)
// 	{

// 		monster_struct *t_monster = monster_manager::get_monster_by_id(monster->data->sight_monster[i]);
// 		if (!t_monster || !t_monster->is_alive())
// 		{
// 			LOG_ERR("%s %d: t_monster[%lu] in sight", __FUNCTION__, __LINE__, monster->data->sight_monster[i]);
// 			continue;
// 		}
// 		struct position * target_pos = t_monster->get_pos();

// 		double pos_x = target_pos->pos_x - monster_pos->pos_x;
// 		double pos_z = target_pos->pos_z - monster_pos->pos_z;
// 		double target_x1 = cos*(pos_x)-sin*(pos_z);
// 		double target_z1 = cos*(pos_z)+sin*(pos_x);

// 		if (target_x1 >= x1 && target_x1 <= x2 && target_z1 >= z1 && target_z1 <= z2)
// 		{
// 			target->push_back(t_monster);
// 		}
// 	}
// }
// static void skill_circle_type(std::vector<unit_struct*> *target, monster_struct* monster, uint64_t skill_id, double radius)
// {
// 	if (NULL == monster)
// 	{
// 		LOG_DEBUG("%s Monster Is Null", __FUNCTION__);
// 		return;
// 	}

// 	struct position *monster_pos = monster->get_pos();
// 	radius = radius * radius;
// 	for (int i = 0; i < monster->data->cur_sight_player; ++i)
// 	{
// 		player_struct *player = player_manager::get_player_by_id(monster->data->sight_player[i]);
// 		//monster_cast_skill_to_player(skill_id, monster, player);
// 		if (!player || !player->is_alive())
// 		{
// 			LOG_ERR("%s %d: player[%lu] in sight", __FUNCTION__, __LINE__, monster->data->sight_player[i]);
// 			continue;
// 		}
// 		double x = monster_pos->pos_x - player->get_pos()->pos_x;
// 		double z = monster_pos->pos_z - player->get_pos()->pos_z;
// 		if (x * x + z * z > radius)
// 			continue;
// 		target->push_back(player);

// 	}

// 	for (int i = 0; i < monster->data->cur_sight_monster; ++i)
// 	{
// 		monster_struct *t_monster = monster_manager::get_monster_by_id(monster->data->sight_monster[i]);
// 		if (!t_monster || !t_monster->is_alive())
// 		{
// 			LOG_ERR("%s %d: t_monster[%lu] in sight", __FUNCTION__, __LINE__, monster->data->sight_monster[i]);
// 			continue;
// 		}
// 		double x = monster_pos->pos_x - t_monster->get_pos()->pos_x;
// 		double z = monster_pos->pos_z - t_monster->get_pos()->pos_z;
// 		if (x * x + z * z > radius)
// 			continue;
// 		target->push_back(t_monster);

// 	}
// }

// static void skill_fan_type(std::vector<unit_struct *> *target, monster_struct* monster, uint64_t skill_id, double radius, double angle)
// {
// 	struct position *monster_pos = monster->get_pos();
// 	radius = radius * radius;
// 	double mmonster_angle = pos_to_angle(monster_pos->pos_x, monster_pos->pos_z);
// 	double angle_min = mmonster_angle - angle;
// 	double angle_max = mmonster_angle + angle;

// 	for (int i = 0; i < monster->data->cur_sight_player; ++i)
// 	{
// 		player_struct *player = player_manager::get_player_by_id(monster->data->sight_player[i]);
// 		//monster_cast_skill_to_player(skill_id, monster, player);
// 		if (!player || !player->is_alive())
// 		{
// 			LOG_ERR("%s %d: player[%lu] in sight", __FUNCTION__, __LINE__, monster->data->sight_player[i]);
// 			continue;
// 		}
// 		double x = monster_pos->pos_x - player->get_pos()->pos_x;
// 		double z = monster_pos->pos_z - player->get_pos()->pos_z;
// 		if (x * x + z * z > radius)
// 			continue;
// 		double angle_target = pos_to_angle(player->get_pos()->pos_x, player->get_pos()->pos_z);
// 		if (angle_target >= angle_min && angle_target <= angle_max)
// 		{
// 			target->push_back(player);
// 		}

// 	}
// 		for (int i = 0; i < monster->data->cur_sight_monster; ++i)
// 		{
// 			monster_struct *t_monster = monster_manager::get_monster_by_id(monster->data->sight_monster[i]);
// 			if (!t_monster || !t_monster->is_alive())
// 			{
// 				LOG_ERR("%s %d: t_monster[%lu] in sight", __FUNCTION__, __LINE__, monster->data->sight_monster[i]);
// 				continue;
// 			}
// 			double x = monster_pos->pos_x - t_monster->get_pos()->pos_x;
// 			double z = monster_pos->pos_z - t_monster->get_pos()->pos_z;
// 			if (x * x + z * z > radius)
// 				continue;
// 			double angle_target = pos_to_angle(t_monster->get_pos()->pos_x, t_monster->get_pos()->pos_z);
// 			if (angle_target >= angle_min && angle_target <= angle_max)
// 			{
// 				target->push_back(t_monster);
// 			}
// 		}
	
// }

static void choose_target(monster_struct* monster, uint32_t skill_id, std::vector<unit_struct *> *target)
{
	
	struct SkillTable *config = get_config_by_id(skill_id, &skill_config);
	if ( NULL == config )
	{
		return;
	}
	
	double length = config->Radius;
	double width = config->Angle;

	assert (config->RangeType == SKILL_RANGE_TYPE_RECT);
	monster->count_rect_unit(monster->data->angle, target, config->MaxCount, length, width, false);
	
	// switch ( config->RangeType )
	// {
	// case SKILL_RANGE_TYPE_RECT:
	// 	skill_rect_type(target, monster, skill_id, length, width);
	// 	break;
	// case SKILL_RANGE_TYPE_CIRCLE:
	// 	skill_circle_type(target, monster, skill_id, length);
	// 	break;
	// case SKILL_RANGE_TYPE_FAN:
	// 	skill_fan_type(target, monster, skill_id, length, width);
	// 	break;
	// default:
	// 	break;
	// }

	
}
static void do_attack(monster_struct* monster)
{
	//寻找攻击目标
	
	std::vector<unit_struct *> target;
	uint32_t skill_id = choose_first_skill(monster);
	if (skill_id == 0)
		return;
	struct SkillTable *config = get_config_by_id(skill_id, &skill_config);
	if (!config)
		return;
	struct ActiveSkillTable *act_config = get_config_by_id(config->SkillAffectId, &active_skill_config);

	uint64_t now = time_helper::get_cached_time();
	if (act_config)
	{
		if (now > monster->ai_data.feijian_ai.trigger_time)
		{
			monster->data->ontick_time = now + 3000;
			monster->ai_state = AI_PATROL_STATE;
			return;
		}

		if ( monster->ai_data.feijian_ai.hurt_flag == 1 )
		{
			monster->ai_data.feijian_ai.hurt_flag = 0;
			if (act_config->ActionTime > 0)
			{
				monster->data->ontick_time = now + act_config->ActionTime;
			}
			else if (act_config->Interval > 0)
			{
				monster->data->ontick_time = now + act_config->Interval;
			}
		
		}
		else
		{
			if (act_config->Interval > 0)
			{
				monster->data->ontick_time = now + act_config->Interval;
			}
		}

		if (act_config->ActionTime <= 0 && act_config->Interval <= 0)
		{
			if (act_config->n_SkillLength >= 3 && (int)act_config->SkillLength[2] >= 0)
			{
				monster->data->ontick_time = now + 3000;
				monster->ai_state = AI_PATROL_STATE;
			}
		}
	}

	choose_target(monster, skill_id, &target);
	if (target.empty())
		return;

	hit_notify_to_many_target(skill_id, monster, &target);
	return;

}
static void do_patrol(monster_struct* monster)
{
	
	uint32_t skill_id = choose_first_skill(monster);
	if (skill_id == 0)
		return;
	struct SkillTable *config = get_config_by_id(skill_id, &skill_config);
	if (!config)
		return;
	struct ActiveSkillTable *act_config = get_config_by_id(config->SkillAffectId, &active_skill_config);
	
	uint64_t now = time_helper::get_cached_time();
	uint64_t m_now = now;
	uint64_t b_now = now;
	if (act_config)
	{
		 for (size_t i = 0; i < act_config->n_SkillLength; ++i)
		 {
			 if (i < 2)
			 {
				 m_now += act_config->SkillLength[i];
			 }
			 if (i < 3)
			 {
				 now += act_config->SkillLength[i];
			 }
		 }

	}
	
	monster->ai_data.feijian_ai.trigger_time = now ;
	monster->ai_data.feijian_ai.hurt_flag = 1;
	monster->ai_state = AI_ATTACK_STATE;
	if (act_config->ActionTime > 0)
	{
		monster->data->ontick_time = m_now + act_config->ActionTime;
	}
	else
	{
		if (b_now != m_now)
		{
			monster->data->ontick_time = m_now;
		}
		
	}	
	//怪物释放技能的方向向量
	float direct_x = 0;
	float direct_z = 1;
	if (monster->create_config)
	{
		if (monster->create_config->Yaw > 270)
		{
			direct_x = 0;
			direct_z = -1;
		}
		
	}

	monster_cast_skill_to_direct(skill_id, monster, direct_x, direct_z, NULL);	
//	for (int i = 0; i < monster->data->cur_sight_player; ++i)
//	{
//		player_struct *player = player_manager::get_player_by_id(monster->data->sight_player[i]);
//
//	}
}

static void ai_tick_8(monster_struct* monster)
{
	if (NULL == monster)
	{
		return;
	}

	monster->data->angle = monster->create_config->Yaw;

	switch (monster->ai_state)
	{
	case AI_ATTACK_STATE:
		do_attack(monster);
		break;
	case AI_PATROL_STATE:  //巡逻
		do_patrol(monster);
		break;
	}
	
}

static void ai_dead_8(monster_struct* monster, scene_struct *scene)
{
	
}

static void ai_befly_8(monster_struct *monster, unit_struct *player)
{

}

static void ai_beattack_8(monster_struct *monster, unit_struct *player)
{


}

struct ai_interface monster_ai_8_interface =
{
	ai_tick_8,
	ai_beattack_8,
	ai_dead_8,
	ai_befly_8,
	NULL,
};
