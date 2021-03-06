#include <assert.h>
#include "game_event.h"
#include "skill.h"
#include "time_helper.h"
#include "game_config.h"
#include "skill_effect.h"

int skill_struct::init_skill(uint32_t id, uint64_t owner, uint64_t target)
{
	assert(owner != 0);
	assert(data);
	std::map<uint64_t, struct SkillTable *>::iterator pos = skill_config.find(id);
	if (pos == skill_config.end())
	{
		LOG_ERR("%s: can not find skill %u config", __FUNCTION__, id);
		return -1;
	}
	config = pos->second;
	data->skill_id = id;
//	data->owner = owner;
//	data->target = target;
	data->lv = 1;
	for (data->fuwen_num = 0; data->fuwen_num < (int)pos->second->n_RuneID; ++data->fuwen_num)
	{
		data->fuwen[data->fuwen_num].id = pos->second->RuneID[data->fuwen_num];
		data->fuwen[data->fuwen_num].lv = 1;
		data->fuwen[data->fuwen_num].isNew = true;
	}
	for (int i = 0; i < MAX_CUR_FUWEN; ++i)
	{
		data->cur_fuwen[i] = 0;
	}
	data->cd_time = 0;
		// TODO: 计算CD

	// struct ActiveSkillTable *active_config = active_skill_config[config->SkillAffectId];
	// if (active_config->ActionTime > 0)
	// {
	// 	data->skill_hit_time = time_helper::get_cached_time() + active_config->ActionTime;
	// 	return (0);
	// }

	
	// 	//立即生效，那么立即计算结果
	// calc_skill_effect();

	return (0);
}

int skill_struct::get_skill_id_and_lv(int fuwen_index, int *id, int *lv)
{
	assert(fuwen_index == 1 || fuwen_index == 2);
	uint32_t fuwen = data->cur_fuwen[fuwen_index - 1];
	if (fuwen == 0)
	{
		*lv = data->lv;
		*id = data->skill_id;
		return (0);
	}
	for (size_t i = 0; i < ARRAY_SIZE(data->fuwen); ++i)
	{
		if (data->fuwen[i].id == fuwen)
		{
			*lv = data->fuwen[i].lv;
			*id = fuwen;
			return 0; 
		}
	}
	*lv = 1;
	*id = fuwen;
	return -1;
}

int skill_struct::add_cd(uint64_t cd)
{
	if (cd != 0)
		data->cd_time = time_helper::get_cached_time() + cd - 200;
	else
		data->cd_time = time_helper::get_cached_time() + config->TotalSkillDelay - 200;
	return (0);
}

// void skill_struct::on_tick()
// {
// 	assert(data);
// 	if (time_helper::get_cached_time() > data->skill_hit_time)
// 		return;
// }

// int skill_struct::check_skill_condition()
// {
// 	return (0);
// }

// int skill_struct::calc_skill_effect()
// {
// /*		
// 	struct SkillLvTable *lv_config = skill_lv_config[config->SkillLv];
// 	for (size_t i = 0; i < lv_config->n_EnemyBuffId; ++i)
// 	{
// 		skill_effect::calc_skill_effect(lv_config->EnemyBuffId[i], 0);
// 	}
// 	for (size_t i = 0; i < lv_config->n_FriendBuffId; ++i)
// 	{
// 		skill_effect::calc_skill_effect(lv_config->FriendBuffId[i], 0);
// 	}
// */	
// 	return (0);
// }

void skill_struct::copy(skill_struct *skill)
{
	config = skill->config;
	assert(data);
	data->skill_id = skill->data->skill_id;
	data->lv = skill->data->lv;
	data->fuwen_num = skill->data->fuwen_num;
	for (int i = 0; i < data->fuwen_num; ++i)
	{
		data->fuwen[i].id = skill->data->fuwen[i].id;
		data->fuwen[i].lv = skill->data->fuwen[i].lv;
		data->fuwen[i].isNew = skill->data->fuwen[i].isNew;
	}
	for (int i = 0; i < MAX_CUR_FUWEN; ++i)
	{
		data->cur_fuwen[i] = skill->data->cur_fuwen[i];
	}
	data->cd_time = 0;
}

fuwen_data *skill_struct::get_fuwen(uint32_t fuwen)
{
	for (int i = 0; i < data->fuwen_num; ++i)
	{
		if (data->fuwen[i].id == fuwen)
		{
			return &data->fuwen[i];
		}
	}
	return NULL;
}

