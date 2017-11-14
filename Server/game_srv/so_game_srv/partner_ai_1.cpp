#include <math.h>
#include <stdlib.h>
#include "camp_judge.h"
#include "game_event.h"
#include "monster_ai.h"
#include "monster_manager.h"
#include "path_algorithm.h"
#include "player_manager.h"
#include "time_helper.h"
#include "partner.h"
#include "check_range.h"
#include "cached_hit_effect.h"
#include "count_skill_damage.h"
#include "msgid.h"
#include "buff.h"

//    ▲ 攻击优先级1：攻击范围内，距离伙伴最近的正在攻击主人的所有目标＞远一些的正在攻击主人的所有目标＞距离伙伴最近的正在攻击伙伴自身的所有目标＞远一些的正在攻击伙伴自身的所有目标＞主人正在攻击的目标
static unit_struct *choose_target(partner_struct *partner)
{
	unit_struct *ret = NULL;
	double distance;
	uint64_t now = time_helper::get_cached_time();
	uint64_t t = now - 60 * 1000;
	struct position *cur_pos = partner->get_pos();

	distance = 0xffffffff;
	for (int i = 0; i < MAX_PARTNER_ATTACK_UNIT; ++i)
	{
		if (partner->attack_owner[i].uuid == 0)
			continue;
		if (partner->attack_owner[i].time <= t)
		{
			partner->attack_owner[i].uuid = 0;
			continue;
		}

		if (!partner->is_unit_in_sight(partner->attack_owner[i].uuid))
			continue;

		unit_struct *target = unit_struct::get_unit_by_uuid(partner->attack_owner[i].uuid);
		if (!target || !target->is_avaliable() || !target->is_alive())
		{
			partner->attack_owner[i].uuid = 0;
			continue;			
		}
		struct position *pos = target->get_pos();
		float x = pos->pos_x - cur_pos->pos_x;
		float z = pos->pos_z - cur_pos->pos_z;		
		if (x * x + z * z <= distance)
		{
			distance = x * x + z * z;
			ret = target;
		}
	}
	if (ret)
		return ret;
	for (int i = 0; i < MAX_PARTNER_ATTACK_UNIT; ++i)
	{
		if (partner->attack_partner[i].uuid == 0)
			continue;
		if (partner->attack_partner[i].time <= t)
		{
			partner->attack_partner[i].uuid = 0;
			continue;
		}
		if (!partner->is_unit_in_sight(partner->attack_partner[i].uuid))
			continue;		
		unit_struct *target = unit_struct::get_unit_by_uuid(partner->attack_partner[i].uuid);
		if (!target || !target->is_avaliable() || !target->is_alive())
		{
			partner->attack_partner[i].uuid = 0;
			continue;			
		}
		struct position *pos = target->get_pos();
		float x = pos->pos_x - cur_pos->pos_x;
		float z = pos->pos_z - cur_pos->pos_z;		
		if (x * x + z * z <= distance)
		{
			distance = x * x + z * z;
			ret = target;
		}
	}
	if (ret)
		return ret;
	for (int i = 0; i < MAX_PARTNER_ATTACK_UNIT; ++i)
	{
		if (partner->owner_attack[i].uuid == 0)
			continue;
		if (partner->owner_attack[i].time <= t)
		{
			partner->owner_attack[i].uuid = 0;
			continue;
		}
		if (!partner->is_unit_in_sight(partner->owner_attack[i].uuid))
			continue;		
		unit_struct *target = unit_struct::get_unit_by_uuid(partner->owner_attack[i].uuid);
		if (!target || !target->is_avaliable() || !target->is_alive())
		{
			partner->owner_attack[i].uuid = 0;
			continue;			
		}
		struct position *pos = target->get_pos();
		float x = pos->pos_x - cur_pos->pos_x;
		float z = pos->pos_z - cur_pos->pos_z;		
		if (x * x + z * z <= distance)
		{
			distance = x * x + z * z;
			ret = target;
		}
	}
	if (ret)
		return ret;
	return NULL;
}

int partner_ai_tick_1(partner_struct *partner)
{
	if (partner->data->skill_id != 0 && partner->ai_state == AI_ATTACK_STATE)
	{
		partner->do_normal_attack();
		partner->data->skill_id = 0;
		partner->ai_state = AI_PATROL_STATE;
		return 1;
	}

	int skill_index;
	uint32_t skill_id = partner->choose_skill(&skill_index);
	if (skill_id == 0)
		return 1;

	if (partner->try_friend_skill(skill_id, skill_index))
	{
		return (0);
	}
	
	unit_struct *target = choose_target(partner);
	if (!target)
		return 0;

	return partner->attack_target(skill_id, skill_index, target);

// 	struct position *my_pos = partner->get_pos();
// 	struct position *his_pos = target->get_pos();
// 	struct SkillTable *config = get_config_by_id(skill_id, &skill_config);
// 	if (config == NULL)
// 	{
// 		LOG_ERR("%s: partner can not find skill[%u] config", __FUNCTION__, skill_id);
// 		return 1;
// 	}
// 	if (!check_distance_in_range(my_pos, his_pos, config->SkillRange))
// 	{
// 			//追击
// 		partner->reset_pos();
// 		if (get_circle_random_position_v2(partner->scene, my_pos, his_pos, config->SkillRange, &partner->data->move_path.pos[1]))		
// 		{
// 			partner->send_patrol_move();
// 		}
// 		else
// 		{
// 			return (0);
// 		}
// 		partner->data->ontick_time += random() % 1000;
// 		return 1;
// 	}

// 		//主动技能
// 	if (config->SkillType == 2)
// 	{
// 		struct ActiveSkillTable *act_config = get_config_by_id(config->SkillAffectId, &active_skill_config);
// 		if (!act_config)
// 		{
// 			LOG_ERR("%s: can not find skillaffectid[%lu] config", __FUNCTION__, config->SkillAffectId);
// 			return 1;
// 		}
// 		if (act_config->ActionTime > 0)
// 		{
// 			uint64_t now = time_helper::get_cached_time();		
// 			partner->data->ontick_time = now + act_config->ActionTime;// + 1500;
// 			partner->data->skill_id = skill_id;
// 			partner->data->angle = -(pos_to_angle(his_pos->pos_x - my_pos->pos_x, his_pos->pos_z - my_pos->pos_z));
// //			LOG_DEBUG("jacktang: mypos[%.2f][%.2f] hispos[%.2f][%.2f]", my_pos->pos_x, my_pos->pos_z, his_pos->pos_x, his_pos->pos_z);
// 			partner->ai_state = AI_ATTACK_STATE;
// 			partner->m_target = target;

// 			partner->reset_pos();
// 			partner->cast_skill_to_target(skill_id, target);		
// 			return 1;
// 		}
// 	}

// 	partner->reset_pos();
// 	partner->cast_immediate_skill_to_target(skill_id, target);

// 		//被反弹死了
// 	if (!partner->data)
// 		return 1;
	
// 	partner->data->skill_id = 0;

// 		//计算硬直时间
// 	partner->data->ontick_time += count_skill_delay_time(config);
	
//	return 1;
}

struct partner_ai_interface partner_ai_1_interface =
{
	.on_tick = partner_ai_tick_1,
	.choose_target = choose_target,
};



