//秋戈的特殊AI
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

static void do_pursue(monster_struct *monster)
{
	if (!monster->data)
		return;

	monster->on_pursue();
			
	if (!monster->target || !monster->target->is_avaliable()
		|| !monster->target->is_alive())
	{
		monster->ai_state = AI_PATROL_STATE;
		return;
	}
	struct position *my_pos = monster->get_pos();
	struct position *his_pos = monster->target->get_pos();

	if (monster->create_config &&
		(fabsf(my_pos->pos_x - monster->get_born_pos_x()) > (int)(monster->ai_config->ChaseRange)
			|| fabsf(my_pos->pos_z - monster->get_born_pos_z()) > (int)(monster->ai_config->ChaseRange)))
	{
		return monster->go_back();
	}

	uint32_t skill_id;
	if (monster->ai_data.qiuge_ai.state != 1)
		skill_id = choose_first_skill(monster);
	else
		skill_id = choose_skill_and_add_cd(monster);
	
	if (skill_id == 0)
		return;
	struct SkillTable *config = get_config_by_id(skill_id, &skill_config);
	if (config == NULL)
	{
		return;
	}

//	LOG_DEBUG("%s: choose skill %u", __FUNCTION__, skill_id)

	if (!check_distance_in_range(my_pos, his_pos, config->SkillRange/*monster->ai_config->ActiveAttackRange*/))
	{
			//追击
//		if (monster->is_unit_in_move())
//			return;
		monster->reset_pos();

//		int direct = getdirection(his_pos, my_pos);
//		if (get_circle_random_position(monster->scene, direct, my_pos, his_pos, /*monster->ai_config->ActiveAttackRange*/config->SkillRange * 5, &monster->data->move_path.pos[1]))
		if (get_circle_random_position_v2(monster->scene, my_pos, his_pos, /*monster->ai_config->ActiveAttackRange*/config->SkillRange, &monster->data->move_path.pos[1]))		
		{
			monster->send_patrol_move();
		}
		else
		{
			monster->go_back();			
		}

		monster->data->ontick_time += random() % 1000;

//		LOG_DEBUG("%s %d: player_pos[%f][%f] move direct[%d] from[%f][%f] to[%f][%f]", __FUNCTION__, __LINE__,
//			his_pos->pos_x, his_pos->pos_z,	direct, my_pos->pos_x, my_pos->pos_z,
//			monster->data->move_path.pos[1].pos_x, monster->data->move_path.pos[1].pos_z);
		
//		monster->data->move_path.pos[1].pos_x = his_pos->pos_x;
//		monster->data->move_path.pos[1].pos_z = his_pos->pos_z;
//		send_patrol_move(monster);
		return;
	}

		//主动技能
	if (config->SkillType == 2)
	{
		struct ActiveSkillTable *act_config = get_config_by_id(config->SkillAffectId, &active_skill_config);
		if (!act_config)
			return;
		if (act_config->ActionTime > 0)
		{
			uint64_t now = time_helper::get_cached_time();		
			monster->data->ontick_time = now + act_config->ActionTime;// + 1500;
//			monster->data->skill_id = skill_id;
//			monster->data->angle = -(pos_to_angle(his_pos->pos_x - my_pos->pos_x, his_pos->pos_z - my_pos->pos_z));
			monster->ai_state = AI_ATTACK_STATE;

			monster->reset_pos();
			monster_cast_skill_to_target(skill_id, monster, monster->target, false);		
			return;
		}
	}

	monster->reset_pos();
	cast_immediate_skill_to_target(skill_id, 1, monster, monster->target);

		//被反弹死了
	if (!monster->data)
		return;
	
	monster->data->skill_id = 0;

		//计算硬直时间
	monster->data->ontick_time += count_skill_delay_time(config);
}

static void ai_tick_18(monster_struct *monster)
{
	if (monster->ai_state == AI_DEAD_STATE)
		return;

	if (monster->is_in_lock_time())
		return;
	if (monster->buff_state & BUFF_STATE_STUN)
		return;

	if (!monster->is_alive())
		assert(0);

	assert(monster->scene);
	area_struct *area = monster->area;
	if (area)
	{
		if (!area->is_all_neighbour_have_player())
			return;
	}

	if (monster->ai_data.qiuge_ai.state != 1 && time_helper::get_cached_time() >= monster->ai_data.qiuge_ai.state1_time)
		monster->ai_data.qiuge_ai.state = 1;
	
	switch (monster->ai_state)
	{
		case AI_ATTACK_STATE:
			do_normal_attack(monster);
			break;
		case AI_PURSUE_STATE:  //追击
			do_pursue(monster);
			break;
		case AI_PATROL_STATE:  //巡逻
			monster->data->ontick_time += monster->count_rand_patrol_time();
			do_normal_patrol(monster);
			break;
	}
}


static void ai_beattack_18(monster_struct *monster, unit_struct *player)
{
}

static void ai_init_18(monster_struct *monster)
{
	monster->ai_data.qiuge_ai.state1_time = time_helper::get_cached_time() + 5000;
}

struct ai_interface monster_ai_18_interface =
{
	ai_tick_18,
	ai_beattack_18,
	normal_ai_dead,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	(ai_init)ai_init_18,
};




