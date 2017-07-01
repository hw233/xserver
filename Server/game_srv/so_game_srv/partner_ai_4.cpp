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

//    ▲ 攻击优先级4：攻击范围内，距离伙伴最近的正在攻击伙伴自己的所有目标＞远一些的正在攻击伙伴自己的所有目标＞距离伙伴最近的正在攻击主人的所有目标＞远一些的正在攻击主人的所有目标＞主人正在攻击的目标
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

int partner_ai_tick_4(partner_struct *partner)
{
	unit_struct *target = choose_target(partner);
	if (!target)
		return 0;
	return 1;
}

struct partner_ai_interface partner_ai_4_interface =
{
	.on_tick = partner_ai_tick_4,
	.choose_target = choose_target,	
};



