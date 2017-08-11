#if 0
#include "boss.h"
#include "monster_manager.h"
#include "time_helper.h"

void boss_struct::init_monster()
{
	monster_struct::init_monster();
	memset(&hate_unit[0], 0, sizeof(hate_unit));
	next_hate_reduce_time = time_helper::get_cached_time() + 1000;

	memset(&skill_cd[0], 0, sizeof(skill_cd));
	
/*	
	for (int i = 0; i < MAX_HATE_UNIT; ++i)
	{
		hate_unit[i].uuid = 0;
		hate_unit[i].hate_value = 0;
	}
*/	
}

UNIT_TYPE boss_struct::get_unit_type()
{
	return UNIT_TYPE_BOSS;
}

// int boss_struct::add_skill_cd(uint32_t index, uint64_t now)
// {
// 	if (index >= MAX_BOSS_SKILL)
// 		return (0);

// 	if (index >= config->n_Skill)
// 		return (0);

// 	uint32_t skill_id = config->Skill[index];
// 	SkillTable *config = get_config_by_id(skill_id, &skill_config);
// 	if (!config)
// 		return (0);
	
// 	struct SkillLvTable *lv_config = get_config_by_id(config->SkillLv, &skill_lv_config);
// 	if (!lv_config)
// 		return (0);
// 	skill_cd[index] = now + lv_config->CD;
// 	LOG_INFO("%s: %p add index[%u] cd[%lu]", __FUNCTION__, this, index, skill_cd[index]);
// 	return (0); 
// }

// bool boss_struct::is_skill_in_cd(uint32_t index, uint64_t now)
// {
// 	if (index >= MAX_BOSS_SKILL)	
// 		return false;
// 	return now < skill_cd[index];
// }

void boss_struct::on_go_back()
{
	target = NULL;
	memset(&hate_unit[0], 0, sizeof(hate_unit));
}
void boss_struct::on_pursue()
{
	uint64_t now = time_helper::get_cached_time();
	if (now < next_hate_reduce_time)
	{
		return;
	}

	next_hate_reduce_time = now + 1000;
	for (int i = 0; i < MAX_HATE_UNIT; ++i)
	{
		if (hate_unit[i].uuid == 0)
			continue;
		hate_unit[i].hate_value *= 0.9;
	}
//	update_target();
}

void boss_struct::reset_timer(uint64_t time)
{
	data->ontick_time = time;
	monster_manager::boss_ontick_reset_timer(this);
}

void boss_struct::set_timer(uint64_t time)
{
	data->ontick_time = time;
	monster_manager::boss_ontick_settimer(this);
}

void boss_struct::count_hate(unit_struct *player, uint32_t skill_id, int32_t damage)
{
	if (damage <= 0)
		return;
	SkillTable *_skill_config = get_config_by_id(skill_id, &skill_config);
	assert(_skill_config);
	double hate_job;
	switch (player->get_job())
	{
		case JOB_DEFINE_MONSTER:
			hate_job = config->HateMonster / 10000.0;
			break;
		case JOB_DEFINE_DAO:
			hate_job = config->HateDao / 10000.0;
			break;
		case JOB_DEFINE_BI:
			hate_job = config->HateBi / 10000.0;
			break;
		case JOB_DEFINE_QIANG:
			hate_job = config->HateQiang / 10000.0;
			break;
		case JOB_DEFINE_FAZHANG:
			hate_job = config->HateFazhang / 10000.0;
			break;
		case JOB_DEFINE_GONG:
			hate_job = config->HateGong / 10000.0;
			break;
	}
	uint64_t add_hate = hate_job * damage * _skill_config->HateAdd / 10000.0 + _skill_config->HateValue;
	uint64_t uuid = player->get_uuid();
	for (int i = 0; i < MAX_HATE_UNIT; ++i)
	{
		if (hate_unit[i].uuid == uuid)
		{
			assert(player->is_avaliable());
			hate_unit[i].hate_value += add_hate;
			return;
		}
	}
	for (int i = 0; i < MAX_HATE_UNIT; ++i)
	{
		if (hate_unit[i].uuid == 0)
		{
			hate_unit[i].uuid = uuid;
			hate_unit[i].hate_value += add_hate;
			return;
		}
	}
}

void boss_struct::update_target()
{
	int max_hate = 0;
	uint64_t target_uuid = 0;
	for (int i = 0; i < MAX_HATE_UNIT; ++i)
	{
		if (hate_unit[i].uuid != 0 && hate_unit[i].hate_value > max_hate)
		{
			max_hate = hate_unit[i].hate_value;
			target_uuid = hate_unit[i].uuid;
		}
	}

	LOG_DEBUG("%s: boss %lu set target = %lu[%d]", __FUNCTION__, get_uuid(), target_uuid, max_hate);
	
	if (target_uuid > 0)
	{
		target = unit_struct::get_unit_by_uuid(target_uuid);
		if (!target)
		{
			LOG_INFO("%s %d: no target %lu", __FUNCTION__, __LINE__, target_uuid);
			return;
		}
		if (!target->is_avaliable())
			target = NULL;
	}
	else
	{
		target = NULL;
	}
}

bool boss_struct::on_unit_leave_sight(uint64_t uuid)
{
	for (int i = 0; i < MAX_HATE_UNIT; ++i)
	{
		if (hate_unit[i].uuid == uuid)
		{
			hate_unit[i].uuid = 0;
			hate_unit[i].hate_value = 0;
			update_target();
			break;
		}
	}
	return true;
}

bool boss_struct::on_monster_leave_sight(uint64_t uuid)
{
//	LOG_DEBUG("%s: %lu: player %lu leave sight", __PRETTY_FUNCTION__, data->player_id, uuid);	
	return on_unit_leave_sight(uuid);
}

bool boss_struct::on_player_leave_sight(uint64_t player_id)
{
//	LOG_DEBUG("%s: %lu: player %lu leave sight", __PRETTY_FUNCTION__, data->player_id, player_id);
	return on_unit_leave_sight(player_id);	
}

void boss_struct::on_beattack(unit_struct *player, uint32_t skill_id, int32_t damage)
{
	if (!data || !scene)
		return;
	
	if (damage > 0)
	{
		count_hate(player, skill_id, damage);
		update_target();
			// TODO: 测试用
		// if (player->get_unit_type() == UNIT_TYPE_PLAYER)
		// {
		// 	char hate_log[256];
		// 	for (int i = 0; i < MAX_HATE_UNIT; ++i)
		// 	{
		// 		if (hate_unit[i].uuid == player->get_uuid())
		// 		{
		// 			sprintf(hate_log, "damage = %d, new hate_value = %.3f", damage, hate_unit[i].hate_value);
		// 			conn_node_gamesrv::send_system_msg(hate_log, dynamic_cast<player_struct *>(player));
		// 			break;
		// 		}
		// 	}
		// }
	}
	
	if (ai && ai->on_beattack)
		ai->on_beattack(this, player);
}

void boss_struct::clear_monster_timer()
{
	if (is_node_in_heap(&monster_manager_m_boss_minheap, this))
		monster_manager::boss_ontick_delete(this);
}
#endif

