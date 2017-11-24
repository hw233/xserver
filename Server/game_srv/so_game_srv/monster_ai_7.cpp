#if 0
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

//停止攻击跑回主人身边的距离
const static int STOP_ATTACK_DISTANCE = 5 * 5;
//需要瞬移到主人身边的距离
const static int FLASH_TO_OWNER_DISTANCE = 15 * 15;

//传送到主人身后的距离
const static int FLASH_DISTANCE1 = 4;
//如果前一个距离上是阻挡，那么第二次尝试的距离
const static int FLASH_DISTANCE2 = 2;


static void ai_dead_7(monster_struct *monster, scene_struct *scene)
{
	monster_manager::delete_monster(monster);
}

static int get_owner_nearby_pos(monster_struct *monster, player_struct *owner, struct position *ret_pos)
{
	struct position *monster_pos, *owner_pos;
	monster_pos = monster->get_pos();
	owner_pos = owner->get_pos();
	double angle = pos_to_angle(monster_pos->pos_x - owner_pos->pos_x, monster_pos->pos_z - owner_pos->pos_z);
//	double angle = pos_to_angle(owner_pos->pos_x - monster_pos->pos_x, owner_pos->pos_z - monster_pos->pos_z);	
	if (get_direct_position_v2(owner->scene, angle, monster_pos, owner_pos, FLASH_DISTANCE1, ret_pos))
	{
//		LOG_DEBUG("%s: angle[%.4f] monster[%.1f][%.1f] owner[%.1f][%.1f] flash to [%.1f][%.1f]", __FUNCTION__, angle,
//			monster_pos->pos_x, monster_pos->pos_z, owner_pos->pos_x, owner_pos->pos_z, ret_pos->pos_x, ret_pos->pos_z);
		return (0);
	}
	else if (get_direct_position_v2(owner->scene, angle, monster_pos, owner_pos, FLASH_DISTANCE2, ret_pos))
	{
		return (0);
	}
	ret_pos->pos_x = owner_pos->pos_x;
	ret_pos->pos_z = owner_pos->pos_z;	
	return 0;
}

static int move_to_owner(monster_struct *monster, player_struct *owner)
{
	if (monster->is_unit_in_move())
		return (0);
	struct position pos, *monster_pos;
	monster_pos = monster->get_pos();
	get_owner_nearby_pos(monster, owner, &pos);
	if (check_can_walk(monster->scene, monster_pos, &pos))
	{
		monster->reset_pos();		
		monster->data->move_path.pos[1].pos_x = pos.pos_x;
		monster->data->move_path.pos[1].pos_z = pos.pos_z;		
		monster->data->move_path.start_time = time_helper::get_cached_time();
		monster->data->move_path.max_pos = 1;
		monster->data->move_path.cur_pos = 0;
		monster->broadcast_monster_move();
	}
	else
	{
	}
	return (0);
}

static int flash_to_owner(monster_struct *monster, player_struct *owner)
{
	struct position pos;
	get_owner_nearby_pos(monster, owner, &pos);
	if (monster->scene)
		monster->scene->delete_monster_from_scene(monster, true);
	monster->set_pos(pos.pos_x, pos.pos_z);
	owner->scene->add_monster_to_scene(monster, 0);
	return (0);
}

static void ai_tick_7(monster_struct *monster)
{
//	LOG_DEBUG("%s %p", __FUNCTION__, monster);
		// 检查生命期
//	if (check_monster_relive(monster))
//	{
//		if (monster->scene)
//			monster->scene->delete_monster_from_scene(monster, true);
//		ai_dead_7(monster);
//		return;
//	}

	player_struct *owner = player_manager::get_player_by_id(monster->data->owner);
	if (!owner)
	{
		LOG_ERR("%s: monster[%p][%lu] do not have owner[%lu]", __FUNCTION__, monster, monster->get_uuid(), monster->data->owner);
		ai_dead_7(monster, monster->scene);
		return;
	}

	if (!owner->scene)
		return;

	if (monster->scene != owner->scene)
	{
		flash_to_owner(monster, owner);
		return;
	}

	int distance = get_unit_distance_square(monster, owner);
	if (distance > FLASH_TO_OWNER_DISTANCE)
	{
		flash_to_owner(monster, owner);
		monster->target = NULL;
		return;
	}
	else if (distance > STOP_ATTACK_DISTANCE)
	{
		move_to_owner(monster, owner);
		monster->target = NULL;		
		return;
	}

		//寻找目标攻击目标
	unit_struct *target = monster->target;
	if (!target)
	{
//		move_to_owner(monster, owner);
		return;
	}

	if (!monster->target->is_avaliable()
		|| !monster->target->is_alive())
	{
		monster->target = NULL;
		return;
	}
	
	uint32_t skill_id = choose_first_skill(monster);
	if (skill_id == 0)
		return;
	
	struct SkillTable *config = get_config_by_id(skill_id, &skill_config);
	if (!config)
	{
//		move_to_owner(monster, owner);
		return;
	}
	cast_immediate_skill_to_target(skill_id, monster, target);
		//计算硬直时间
	monster->data->ontick_time += count_skill_delay_time(config);	
}

//巡逻中被攻击则进入追击
static void ai_beattack_7(monster_struct *monster, unit_struct *player)
{
}

//被击飞击退击倒
static void ai_befly_7(monster_struct *monster, unit_struct *player)
{
}

static void ai_alive_7(monster_struct *monster)
{
	monster->data->relive_time = monster->ai_config->Regeneration * 1000 + time_helper::get_cached_time();
}

static void ai_owner_attack_7(monster_struct *monster, player_struct *owner, unit_struct *target)
{
	if (target->is_alive())
		monster->target = target;
}

static void ai_owner_beattack_7(monster_struct *monster, player_struct *owner, unit_struct *attacker)
{
}

struct ai_interface monster_ai_7_interface =
{
	ai_tick_7,
	ai_beattack_7,
	ai_dead_7,
	ai_befly_7,
	ai_alive_7,
	NULL,
	ai_owner_attack_7,
	ai_owner_beattack_7,	
};
#endif





