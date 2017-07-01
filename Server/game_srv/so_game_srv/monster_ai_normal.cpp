#include <math.h>
#include <stdlib.h>
#include "camp_judge.h"
#include "game_event.h"
#include "monster_ai.h"
#include "monster_manager.h"
#include "path_algorithm.h"
#include "player_manager.h"
#include "time_helper.h"
#include "unit.h"
#include "check_range.h"
#include "cached_hit_effect.h"
#include "count_skill_damage.h"
#include "msgid.h"
#include "buff.h"

// static void try_attack_target(monster_struct *monster, struct SkillTable *config)
// {
// 	struct position *my_pos = monster->get_pos();
// 	struct position *his_pos = monster->target->get_pos();

// 	assert(config && config->SkillType == 2);
// //	LOG_DEBUG("%s monster hit skill %u", __FUNCTION__, config->ID);
// 	if (check_distance_in_range(my_pos, his_pos, config->SkillRange/*monster->ai_config->ActiveAttackRange*/))
// 	{
// 		monster_hit_notify_to_player(config->ID, monster, monster->target);
// 	}
// }

// static void try_attack(monster_struct *monster, struct SkillTable *config)
// {
// 	assert(config && config->SkillType == 2);
	
// 	std::vector<unit_struct *> target;
// 	if (monster->count_skill_hit_unit(&target, config, monster->target) != 0)
// 		return;
// 	monster_hit_notify_to_many_player(config->ID, monster, NULL, &target);
// }

// static void do_attack(monster_struct *monster)
// {
// 	if (!monster->data)
// 		return;
// //	if (monster->ai_type != AI_TYPE_NORMAL)
// //		return;
// 	if (!monster->target || !monster->target->is_avaliable())
// 	{
// 		monster->ai_state = AI_PATROL_STATE;
// 		return;
// 	}

// 	struct SkillTable *config = get_config_by_id(monster->data->skill_id, &skill_config);
	
// 	monster->reset_pos();	
// 	monster->ai_state = AI_PURSUE_STATE;
// 	monster->data->skill_id = 0;
	
// 	if (!config)
// 		return;
// 	struct ActiveSkillTable *act_config = get_config_by_id(config->SkillAffectId, &active_skill_config);
// 	if (act_config)
// 	{
// 		monster->data->ontick_time += act_config->TotalSkillDelay;
// 		// for (size_t i = 0; i < act_config->n_SkillLength; ++i)
// 		// {
// 		// 	monster->data->ontick_time += act_config->SkillLength[i];
// 		// }
// 	}

// 	if (config->MaxCount <= 1)
// 	{
// 		try_attack_target(monster, config);
// 	}
// 	else
// 	{
// 		try_attack(monster, config);
// 	}
// }

// static void do_patrol(monster_struct *monster)
// {
// 	if (!monster->config)
// 		return;
// //	if (monster->ai_type != AI_TYPE_NORMAL)
// //		return;
	
// 	struct position *cur_pos = monster->get_pos();
	
// 	if (monster->create_config &&
// 		(fabsf(cur_pos->pos_x - monster->get_born_pos_x()) > (int)(monster->ai_config->GuardRange)
// 			|| fabsf(cur_pos->pos_z - monster->get_born_pos_z()) > (int)(monster->ai_config->GuardRange)))
// 	{
// 		return monster->go_back();
// 	}
	
// 	if (monster->is_unit_in_move())
// 		return;

// 	if (monster->try_active_attack())
// 	{
// 		monster->ai_state = AI_PURSUE_STATE;
// //	uint64_t now = time_helper::get_cached_time();			
// //	monster->reset_timer(now + 500);
// 		return;
// 	}
	
// //	monster->data->move_path.pos[0].pos_x = monster->data->move_path.pos[monster->data->move_path.cur_pos].pos_x;
// //	monster->data->move_path.pos[0].pos_z = monster->data->move_path.pos[monster->data->move_path.cur_pos].pos_z;
// 	monster->reset_pos();
// 	if (find_rand_position(monster->scene, &monster->data->move_path.pos[0],
// 			monster->ai_config->GuardRange, &monster->data->move_path.pos[1]) == 0)
// 	{
// 		monster->send_patrol_move();		
// 	}
// }

// static void	do_pursue(monster_struct *monster)
// {
// 	if (!monster->data)
// 		return;
// //	if (monster->ai_type != AI_TYPE_NORMAL)
// //		return;

// 	monster->on_pursue();
			
// 	if (!monster->target || !monster->target->is_avaliable()
// 		|| !monster->target->is_alive())
// 	{
// 		monster->ai_state = AI_PATROL_STATE;
// 		return;
// 	}
// 	struct position *my_pos = monster->get_pos();
// 	struct position *his_pos = monster->target->get_pos();

// 	if (monster->create_config &&
// 		(fabsf(my_pos->pos_x - monster->get_born_pos_x()) > (int)(monster->ai_config->ChaseRange)
// 			|| fabsf(my_pos->pos_z - monster->get_born_pos_z()) > (int)(monster->ai_config->ChaseRange)))
// 	{
// 		return do_go_back(monster);
// 	}

// //	uint32_t skill_id = choose_rand_skill(monster);
// 	uint32_t skill_id = choose_skill_and_add_cd(monster);
// 	if (skill_id == 0)
// 		return;
// 	struct SkillTable *config = get_config_by_id(skill_id, &skill_config);
// 	if (config == NULL)
// 	{
// 		return;
// 	}

// //	LOG_DEBUG("%s: choose skill %u", __FUNCTION__, skill_id)

// 	if (!check_distance_in_range(my_pos, his_pos, config->SkillRange/*monster->ai_config->ActiveAttackRange*/))
// 	{
// 			//追击
// //		if (monster->is_unit_in_move())
// //			return;
// 		monster->reset_pos();

// //		int direct = getdirection(his_pos, my_pos);
// //		if (get_circle_random_position(monster->scene, direct, my_pos, his_pos, /*monster->ai_config->ActiveAttackRange*/config->SkillRange * 5, &monster->data->move_path.pos[1]))
// 		if (get_circle_random_position_v2(monster->scene, my_pos, his_pos, /*monster->ai_config->ActiveAttackRange*/config->SkillRange, &monster->data->move_path.pos[1]))		
// 		{
// 			monster->send_patrol_move();
// 		}
// 		else
// 		{
// 			do_go_back(monster);			
// 		}

// 		monster->data->ontick_time += random() % 1000;

// //		LOG_DEBUG("%s %d: player_pos[%f][%f] move direct[%d] from[%f][%f] to[%f][%f]", __FUNCTION__, __LINE__,
// //			his_pos->pos_x, his_pos->pos_z,	direct, my_pos->pos_x, my_pos->pos_z,
// //			monster->data->move_path.pos[1].pos_x, monster->data->move_path.pos[1].pos_z);
		
// //		monster->data->move_path.pos[1].pos_x = his_pos->pos_x;
// //		monster->data->move_path.pos[1].pos_z = his_pos->pos_z;
// //		send_patrol_move(monster);
// 		return;
// 	}

// 		//主动技能
// 	if (config->SkillType == 2)
// 	{
// 		struct ActiveSkillTable *act_config = get_config_by_id(config->SkillAffectId, &active_skill_config);
// 		if (!act_config)
// 			return;
// 		if (act_config->ActionTime > 0)
// 		{
// 			uint64_t now = time_helper::get_cached_time();		
// 			monster->data->ontick_time = now + act_config->ActionTime;// + 1500;
// 			monster->data->skill_id = skill_id;
// 			monster->data->angle = -(pos_to_angle(his_pos->pos_x - my_pos->pos_x, his_pos->pos_z - my_pos->pos_z));
// //			LOG_DEBUG("jacktang: mypos[%.2f][%.2f] hispos[%.2f][%.2f]", my_pos->pos_x, my_pos->pos_z, his_pos->pos_x, his_pos->pos_z);
// 			monster->ai_state = AI_ATTACK_STATE;

// 			monster->reset_pos();
// 			monster_cast_skill_to_player(skill_id, monster, monster->target);		
// 			return;
// 		}
// 	}

// 	monster->reset_pos();
// 	monster_cast_immediate_skill_to_player(skill_id, monster, NULL, monster->target);

// 		//被反弹死了
// 	if (!monster->data)
// 		return;
	
// 	monster->data->skill_id = 0;

// 		//计算硬直时间
// 	monster->data->ontick_time += count_skill_delay_time(config);
// }

static void	do_goback(monster_struct *monster)
{
	if (!monster->create_config)
		return;
	monster->reset_pos();
	monster->data->move_path.pos[1].pos_x = monster->get_born_pos_x();
	monster->data->move_path.pos[1].pos_z = monster->get_born_pos_z();
	monster->send_patrol_move();
}

static void	do_dead(monster_struct *monster)
{
	if (!monster->create_config)
		return;	
	if (check_monster_relive(monster))
	{
		monster->target = NULL;
		monster->ai_state = AI_PATROL_STATE;
		monster->set_pos(monster->get_born_pos_x(),	monster->get_born_pos_z());
		monster->data->attrData[PLAYER_ATTR_HP] = monster->data->attrData[PLAYER_ATTR_MAXHP];
		monster->on_relive();
	}
}

void normal_ai_tick(monster_struct *monster)
{
	if (monster->ai_state == AI_DEAD_STATE)
		return do_dead(monster);

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
	switch (monster->ai_state)
	{
		case AI_ATTACK_STATE:
			do_normal_attack(monster);
			break;
		case AI_PURSUE_STATE:  //追击
			do_normal_pursue(monster);
			break;
		case AI_GO_BACK_STATE:
			monster->data->ontick_time += random() % 2000;
			do_goback(monster);
			break;
		case AI_PATROL_STATE:  //巡逻
			monster->data->ontick_time += monster->count_rand_patrol_time();
			do_normal_patrol(monster);
			break;
	}
}

//巡逻中被攻击则进入追击
static void normal_ai_beattack(monster_struct *monster, unit_struct *player)
{
	if (monster->ai_state != AI_PATROL_STATE)
		return;
		
	monster->ai_state = AI_PURSUE_STATE;
	monster->data->target_pos.pos_x = 0;
	monster->data->target_pos.pos_z = 0;		
	if (monster->target && monster->target->is_avaliable())
		return;
	monster->target = player;

	uint64_t now = time_helper::get_cached_time();			
	monster->reset_timer(now + 500);
//	monster_manager::monster_ontick_reset_timer(monster);
}

//被击飞击退击倒
static void normal_ai_befly(monster_struct *monster, unit_struct *player)
{
		//打断技能
	if (monster->ai_state == AI_ATTACK_STATE)
	{
		monster->ai_state = AI_PURSUE_STATE;
		monster->data->target_pos.pos_x = 0;
		monster->data->target_pos.pos_z = 0;			
		monster->data->skill_id = 0;		
	}
}

static void normal_ai_dead(monster_struct *monster, scene_struct *scene)
{
	monster->ai_state = AI_DEAD_STATE;
}

static bool	normal_ai_player_leave_sight(monster_struct *monster, player_struct *player)
{
	if (monster->target && monster->target->get_uuid() == player->data->player_id)
		monster->target = NULL;
	return true;
}

struct ai_interface monster_ai_normal_interface =
{
	normal_ai_tick,
	normal_ai_beattack,
	normal_ai_dead,
	normal_ai_befly,
	NULL,
	normal_ai_player_leave_sight,
};




