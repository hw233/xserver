#include <math.h>
#include "uuid.h"
#include "monster_ai.h"
#include "game_event.h"
#include "camp_judge.h"
#include "monster_ai.h"
#include "path_algorithm.h"
#include "player_manager.h"
#include "monster_manager.h"
#include "time_helper.h"
#include "unit.h"
#include "raid.h"
#include "buff_manager.h"
#include "check_range.h"
#include "cached_hit_effect.h"
#include "count_skill_damage.h"
#include "msgid.h"

#define CALL_SKILL_RADIUS (10)

//static void try_attack_target(monster_struct *monster, struct SkillTable *config);
//static void try_attack(monster_struct *monster, struct SkillTable *config);

uint32_t count_skill_delay_time(struct SkillTable *config)
{
	int ret = 0;
	if (config == NULL)
		return (0);
		//普通攻击或者主动攻击, 计算硬直时间
	if (config->SkillType == 1 || config->SkillType == 2)
	{
//		struct ActiveSkillTable *act_config = get_config_by_id(config->SkillAffectId, &active_skill_config);
//		if (act_config)
		{
			ret += config->TotalSkillDelay + random() % 1000; //(500 * act_config->n_SkillLength);
			// for (size_t i = 0; i < act_config->n_SkillLength; ++i)
			// {
			// 	ret += act_config->SkillLength[i] + random() % 500;
			// }
		}
	}
	return ret;
}

// uint32_t choose_rand_skill(monster_struct *monster)
// {
// 	if (monster->data->next_skill_id > 0)
// 	{
// 		uint32_t ret = monster->data->next_skill_id;
// 		monster->data->next_skill_id = 0;
// 		return ret;
// 	}
	
// 	if (monster->data->skill_id > 0)
// 		return monster->data->skill_id;
// 	if (monster->config->n_Skill <= 0)
// 		return (0);
// 	int n = random() % monster->config->n_Skill;
// 	return monster->config->Skill[n];
// }

uint32_t choose_skill_and_add_cd(monster_struct *monster)
{
	if (monster->config->n_Skill == 0)
		return (0);
	
	uint64_t now = time_helper::get_cached_time();
	if (monster->data->next_skill_id > 0)
	{
		uint32_t ret = monster->data->next_skill_id;		
		monster->data->next_skill_id = 0;		
		for (int i = monster->config->n_Skill - 1; i >= 0; --i)
		{
			uint32_t skill_id = monster->config->Skill[i];
			if (skill_id != ret)
				continue;
			monster->add_skill_cd(i, now);			
			break;
		}
		return ret;
	}

	for (uint32_t i = 1; i < monster->config->n_Skill; ++i)	
//	for (int i = monster->config->n_Skill - 1; i >= 0; --i)
	{
		if (monster->is_skill_in_cd(i, now))
			continue;
		monster->add_skill_cd(i, now);
		return monster->config->Skill[i];		
	}

	if (!monster->is_skill_in_cd(0, now) && monster->config->n_Skill > 0)
	{
		monster->add_skill_cd(0, now);
		return monster->config->Skill[0];				
	}
	
	return (0);
}

uint32_t choose_first_skill(monster_struct *monster)
{
	if (monster->config->n_Skill == 0)
		return (0);
	return monster->config->Skill[0];
}
/*
void send_patrol_move(monster_struct *monster)
{
	monster->data->move_path.start_time = time_helper::get_cached_time();
	monster->data->move_path.max_pos = 1;
	monster->data->move_path.cur_pos = 0;
	monster->broadcast_monster_move();
}
*/
void hit_notify_to_many_target(uint64_t skill_id, unit_struct *attack, std::vector<unit_struct *> *target)
{
	int n_hit_effect = 0;
	int n_buff = 0;
//	double *target_attr;

	if (target->empty())
		return;
/*
	struct SkillLvTable *lv_config1, *lv_config2;
	get_skill_lv_config(skill_id, &lv_config1, &lv_config2);
	if (!lv_config1 && !lv_config2)
	{
		LOG_ERR("%s %d: skill[%u] no config", __FUNCTION__, __LINE__, skill_id);
		return;
	}
*/
//	struct SkillLvTable *lv_config1, *lv_config2;
//	struct PassiveSkillTable *pas_config;
	struct SkillTable *ski_config = get_config_by_id(skill_id, &skill_config);
	if (!ski_config)
	{
		LOG_ERR("%s %d: skill[%lu] no config", __FUNCTION__, __LINE__, skill_id);
		return;		
	}
//	get_skill_configs(1, skill_id, &ski_config, &lv_config1, &pas_config, &lv_config2, NULL);
	// if (!lv_config1 && !lv_config2)
	// {
	// 	LOG_ERR("%s %d: skill[%lu] no config", __FUNCTION__, __LINE__, skill_id);
	// 	return;
	// }

	uint32_t life_steal = 0;
	uint32_t damage_return = 0;
	player_struct *owner = attack->get_owner();

	for (std::vector<unit_struct *>::iterator ite = target->begin(); ite != target->end(); ++ite)
	{
		unit_struct *player = *ite;
		assert(player->is_avaliable());

//		if (!check_can_attack(monster, player))
		if (owner)
		{
			if (get_unit_fight_type(owner, player) != UNIT_FIGHT_TYPE_ENEMY)
				continue;
		}
		else 
		{
			if (get_unit_fight_type(attack, player) != UNIT_FIGHT_TYPE_ENEMY)								
				continue;			
		}
		
		if (!player->is_alive())
		{
			continue;
		}

		if (player->is_too_high_to_beattack())
			continue;
		
		cached_hit_effect_point[n_hit_effect] = &cached_hit_effect[n_hit_effect];
		skill_hit_effect__init(&cached_hit_effect[n_hit_effect]);
		uint32_t add_num = 0;
		int32_t damage = 0;
		int32_t other_rate = count_other_skill_damage_effect(attack, player);						
		damage += count_skill_total_damage(UNIT_FIGHT_TYPE_ENEMY, ski_config, 
			1, attack, player,
			&cached_hit_effect[n_hit_effect].effect,
			&cached_buff_id[n_buff],
			&cached_buff_end_time[n_buff],
			&add_num, other_rate);

		life_steal += attack->count_life_steal_effect(damage);
		damage_return += attack->count_damage_return(damage, player);

		if (player->get_unit_type() == UNIT_TYPE_PLAYER)
		{
			player_struct *t = ((player_struct *)player);
			raid_struct *raid = t->get_raid();
			if (raid && get_entity_type(attack->get_uuid()) == ENTITY_TYPE_MONSTER)
			{
				raid->on_monster_attack((monster_struct *)attack, t, damage);
			}

			if (owner)
			{
				check_qiecuo_finished(owner, t);
			}
		}


		LOG_DEBUG("%s: unit[%lu][%p] damage[%d] hp[%f]", __FUNCTION__, player->get_uuid(), player, damage, player->get_attr(PLAYER_ATTR_HP));

		cached_hit_effect[n_hit_effect].playerid = player->get_uuid();
		cached_hit_effect[n_hit_effect].n_add_buff = add_num;
		cached_hit_effect[n_hit_effect].add_buff = &cached_buff_id[n_buff];
		cached_hit_effect[n_hit_effect].hp_delta = damage;
		cached_hit_effect[n_hit_effect].cur_hp = player->get_attr(PLAYER_ATTR_HP);
//		cached_hit_effect[n_hit_effect].attack_pos = &cached_attack_pos[n_hit_effect];
		cached_hit_effect[n_hit_effect].target_pos = &cached_target_pos[n_hit_effect];	
//		pos_data__init(&cached_attack_pos[n_hit_effect]);
		pos_data__init(&cached_target_pos[n_hit_effect]);
		
//		cached_attack_pos[n_hit_effect].pos_x = monster->get_pos()->pos_x;
//		cached_attack_pos[n_hit_effect].pos_z = monster->get_pos()->pos_z;		
		cached_target_pos[n_hit_effect].pos_x = player->get_pos()->pos_x;
		cached_target_pos[n_hit_effect].pos_z = player->get_pos()->pos_z;
		
		n_buff += add_num;
		++n_hit_effect;

		if (player->is_alive())
		{
			player->on_beattack(attack, skill_id, damage);						
		}
		else
		{
			if (owner)
			{
				player->on_dead(owner);
			}
			else
			{
				player->on_dead(attack);
			}
		}

	}

	if (n_hit_effect == 0)
		return;
	
	target->push_back(attack);

	SkillHitNotify notify;
	skill_hit_notify__init(&notify);
	notify.playerid = attack->get_uuid();

	if (owner)
	{
		notify.owneriid = owner->get_uuid();
		notify.ownername = owner->get_name();
	}
	else
	{
		notify.owneriid = notify.playerid;
	}
	
	notify.skillid = skill_id;
	notify.n_target_player = n_hit_effect;
	notify.target_player = cached_hit_effect_point;

	notify.attack_cur_hp = attack->get_attr(PLAYER_ATTR_HP);
	notify.life_steal = life_steal;
	notify.damage_return = damage_return;

	PosData attack_pos;
	pos_data__init(&attack_pos);
	attack_pos.pos_x = attack->get_pos()->pos_x;
	attack_pos.pos_z = attack->get_pos()->pos_z;
	notify.attack_pos = &attack_pos;
	player_struct::broadcast_to_many_sight(MSG_ID_SKILL_HIT_NOTIFY, &notify, (pack_func)skill_hit_notify__pack, *target);
	
	if (!attack->is_alive())
	{
		attack->on_dead(attack);
	}
}

void hit_notify_to_many_friend(uint64_t skill_id, unit_struct *monster, std::vector<unit_struct *> *target)
{
	int n_hit_effect = 0;
	int n_buff = 0;

	if (target->empty())
		return;

//	struct SkillLvTable *lv_config1, *lv_config2;
//	struct PassiveSkillTable *pas_config;
	struct SkillTable *ski_config = get_config_by_id(skill_id, &skill_config);
//	get_skill_configs(1, skill_id, &ski_config, &lv_config1, &pas_config, &lv_config2, NULL);
//	if (!lv_config1 && !lv_config2)
	if (!ski_config)
	{
		LOG_ERR("%s %d: skill[%lu] no config", __FUNCTION__, __LINE__, skill_id);
		return;
	}

	for (std::vector<unit_struct *>::iterator ite = target->begin(); ite != target->end(); ++ite)
	{
		unit_struct *player = *ite;
		assert(player->is_avaliable());

		if (!player->is_alive())
		{
			continue;
		}

		if (player->is_too_high_to_beattack())
			continue;
		
		cached_hit_effect_point[n_hit_effect] = &cached_hit_effect[n_hit_effect];
		skill_hit_effect__init(&cached_hit_effect[n_hit_effect]);
		uint32_t add_num = 0;
		int32_t damage = 0;
		int32_t other_rate = count_other_skill_damage_effect(monster, player);						
		damage += count_skill_total_damage(UNIT_FIGHT_TYPE_FRIEND, ski_config,
			1, monster, player,
			&cached_hit_effect[n_hit_effect].effect,
			&cached_buff_id[n_buff],
			&cached_buff_end_time[n_buff],
			&add_num, other_rate);

		LOG_DEBUG("%s: unit[%lu][%p] damage[%d] hp[%f]", __FUNCTION__, player->get_uuid(), player, damage, player->get_attr(PLAYER_ATTR_HP));

		cached_hit_effect[n_hit_effect].playerid = player->get_uuid();
		cached_hit_effect[n_hit_effect].n_add_buff = add_num;
		cached_hit_effect[n_hit_effect].add_buff = &cached_buff_id[n_buff];
		cached_hit_effect[n_hit_effect].hp_delta = -damage;
		cached_hit_effect[n_hit_effect].cur_hp = player->get_attr(PLAYER_ATTR_HP);
		cached_hit_effect[n_hit_effect].target_pos = &cached_target_pos[n_hit_effect];	
		pos_data__init(&cached_target_pos[n_hit_effect]);
		
		cached_target_pos[n_hit_effect].pos_x = player->get_pos()->pos_x;
		cached_target_pos[n_hit_effect].pos_z = player->get_pos()->pos_z;
		
		n_buff += add_num;
		++n_hit_effect;
	}

	if (n_hit_effect == 0)
		return;
	
	target->push_back(monster);

	SkillHitNotify notify;
	skill_hit_notify__init(&notify);
	notify.playerid = monster->get_uuid();

	notify.owneriid = notify.playerid;
	
	notify.skillid = skill_id;
	notify.n_target_player = n_hit_effect;
	notify.target_player = cached_hit_effect_point;

	notify.attack_cur_hp = monster->get_attr(PLAYER_ATTR_HP);

	PosData attack_pos;
	pos_data__init(&attack_pos);
	attack_pos.pos_x = monster->get_pos()->pos_x;
	attack_pos.pos_z = monster->get_pos()->pos_z;
	notify.attack_pos = &attack_pos;
	player_struct::broadcast_to_many_sight(MSG_ID_SKILL_HIT_NOTIFY, &notify, (pack_func)skill_hit_notify__pack, *target);
}

void hit_notify_to_target(uint64_t skill_id, unit_struct *attack, unit_struct *target)
{
	std::vector<unit_struct *> t;
	t.push_back(target);
	hit_notify_to_many_target(skill_id, attack, &t);
}

void monster_cast_call_monster_skill(monster_struct *monster, uint64_t skill_id)
{
	SkillTable *config = get_config_by_id(skill_id, &skill_config);
	if (!config)
		return;
	SkillLvTable *lv_config = get_config_by_id(config->SkillLv, &skill_lv_config);
	if (lv_config && lv_config->MonsterID != 0 && lv_config->MonsterLv != 0)
	{
		monster_cast_skill_to_target(monster->ai_data.leixinye_ai.call_skill_id, monster, NULL, false);
		struct position *t_pos = monster->get_pos();
		monster_manager::create_monster_at_pos(monster->scene, lv_config->MonsterID,
			lv_config->MonsterLv, t_pos->pos_x, t_pos->pos_z, lv_config->MonsterEff, monster, 0);
	}	
}

void monster_try_skill_talk(monster_struct *monster, uint64_t skill_id)
{
	if (!monster->config->talk_config)
		return;
	struct NpcTalkTable *talk_config;
	for (talk_config = monster->config->talk_config; talk_config; talk_config = talk_config->next)
	{
		if (talk_config->Type != 2)
			continue;
		if (talk_config->EventNum1 != 1)
			continue;
		assert(talk_config->n_EventNum2 > 0);
		if (skill_id != talk_config->EventNum2[0])
			continue;
		MonsterTalkNotify nty;
		monster_talk_notify__init(&nty);
		nty.talkid = talk_config->ID;
		nty.uuid = monster->get_uuid();
		monster->broadcast_to_sight(MSG_ID_MONSTER_TALK_NOTIFY, &nty, (pack_func)monster_talk_notify__pack, false);
		return;
	}
}

void monster_cast_skill_to_target(uint64_t skill_id, monster_struct *monster, unit_struct *target, bool use_target_pos)
{
	float x = 0;
	float z = 0;
	struct position *target_pos = NULL;
	if (target)
	{
		struct position *pos = monster->get_pos();
		struct position *player_pos = target->get_pos();
		x = player_pos->pos_x - pos->pos_x;
		z = player_pos->pos_z - pos->pos_z;

		if (use_target_pos)
		{
			target_pos = player_pos;
			monster->data->skill_target_pos.pos_x = player_pos->pos_x;
			monster->data->skill_target_pos.pos_z = player_pos->pos_z;		
		}
		monster->data->skill_id = skill_id;
		monster->data->angle = -(pos_to_angle(player_pos->pos_x - pos->pos_x, player_pos->pos_z - pos->pos_z));		
		
	}
	
	monster_cast_skill_to_direct(skill_id, monster,	x, z, target_pos);
	// monster_try_skill_talk(monster, skill_id);
	
	// SkillCastNotify notify;
	// skill_cast_notify__init(&notify);
	// notify.skillid = skill_id;
	// notify.playerid = monster->data->player_id;
	// PosData cur_pos;
	// pos_data__init(&cur_pos);
	// struct position *pos = monster->get_pos();
	// cur_pos.pos_x = pos->pos_x;
	// cur_pos.pos_z = pos->pos_z;		
	// notify.cur_pos = &cur_pos;

	// PosData target_pos;

	// if (player)
	// {
	// 	struct position *player_pos = player->get_pos();
	// 	notify.direct_x = player_pos->pos_x - pos->pos_x;
	// 	notify.direct_z = player_pos->pos_z - pos->pos_z;

	// 	if (use_target_pos)
	// 	{
	// 		pos_data__init(&target_pos);
	// 		target_pos.pos_x = player_pos->pos_x;
	// 		target_pos.pos_z = player_pos->pos_z;			
	// 		notify.target_pos = &target_pos;
	// 	}
	// }
	// monster->broadcast_to_sight(MSG_ID_SKILL_CAST_NOTIFY, &notify, (pack_func)skill_cast_notify__pack, false);
}

void cast_immediate_skill_to_target(uint64_t skill_id, uint32_t skill_lv, unit_struct *attack, unit_struct *target)
{
	player_struct *a_owner = attack->get_owner();
	player_struct *d_owner = target->get_owner();

	struct position *a_pos = attack->get_pos();
	struct position *d_pos = target->get_pos();

	unit_struct *a = a_owner ? a_owner : attack;
	unit_struct *d = d_owner ? d_owner : target;	

	if (get_unit_fight_type(a, d) != UNIT_FIGHT_TYPE_ENEMY)
		return;

//	if (player->buff_state & BUFF_STATE_GOD)
//	{
//		return;
//	}
	
	int n_hit_effect = 0;
	int n_buff = 0;
	assert(target && target->is_avaliable());
	cached_hit_effect_point[n_hit_effect] = &cached_hit_effect[n_hit_effect];
	skill_hit_effect__init(&cached_hit_effect[n_hit_effect]);
	uint32_t add_num = 0;
	int32_t damage = 0;

//	struct SkillLvTable *lv_config1, *lv_config2;
//	struct PassiveSkillTable *pas_config;
	struct SkillTable *ski_config = get_config_by_id(skill_id, &skill_config);
//	get_skill_configs(skill_lv, skill_id, &ski_config, &lv_config1, &pas_config, &lv_config2, NULL);
//	if (!lv_config1 && !lv_config2)
	if (!ski_config)	
	{
		LOG_ERR("%s %d: skill[%lu] no config", __FUNCTION__, __LINE__, skill_id);
		return;
	}
	
	int32_t other_rate = count_other_skill_damage_effect(attack, target);					
	damage += count_skill_total_damage(UNIT_FIGHT_TYPE_ENEMY, ski_config,
		1, attack, target,
		&cached_hit_effect[n_hit_effect].effect,
		&cached_buff_id[n_buff],
		&cached_buff_end_time[n_buff],
		&add_num, other_rate);

	target->on_hp_changed(damage);

	if (target->get_unit_type() == UNIT_TYPE_PLAYER
		&& get_entity_type(attack->get_uuid()) == ENTITY_TYPE_MONSTER)
	{
		player_struct *t = ((player_struct *)target);
		raid_struct *raid = t->get_raid();
		if (raid)
			raid->on_monster_attack((monster_struct *)attack, t, damage);
	}

	LOG_DEBUG("%s: unit[%lu][%p] attack unit[%lu][%p] damage[%d] hp[%f]",
		__FUNCTION__, attack->get_uuid(), attack,
		target->get_uuid(), target, damage, target->get_attr(PLAYER_ATTR_HP));

	uint32_t life_steal = attack->count_life_steal_effect(damage);
	uint32_t damage_return = attack->count_damage_return(damage, target);

	attack->on_hp_changed(damage_return);

	PosData attack_pos;
	pos_data__init(&attack_pos);
	attack_pos.pos_x = a_pos->pos_x;
	attack_pos.pos_z = a_pos->pos_z;
	
	cached_hit_effect[n_hit_effect].playerid = target->get_uuid();
	cached_hit_effect[n_hit_effect].n_add_buff = add_num;
	cached_hit_effect[n_hit_effect].add_buff = &cached_buff_id[n_buff];
//	cached_hit_effect[n_hit_effect].add_buff_end_time = &cached_buff_end_time[n_buff];	
	cached_hit_effect[n_hit_effect].hp_delta = damage;
	cached_hit_effect[n_hit_effect].cur_hp = target->get_attr(PLAYER_ATTR_HP);
//	cached_hit_effect.attack_pos = &attack_pos;
	cached_hit_effect[n_hit_effect].target_pos = &cached_target_pos[n_hit_effect];	
//	pos_data__init(&cached_attack_pos[n_hit_effect]);
	pos_data__init(&cached_target_pos[n_hit_effect]);	
//	cached_attack_pos[n_hit_effect].pos_x = monster->get_pos()->pos_x;
//	cached_attack_pos[n_hit_effect].pos_z = monster->get_pos()->pos_z;
	cached_target_pos[n_hit_effect].pos_x = d_pos->pos_x;
	cached_target_pos[n_hit_effect].pos_z = d_pos->pos_z;			
	
	n_buff += add_num;
	++n_hit_effect;

	SkillHitImmediateNotify notify;
	skill_hit_immediate_notify__init(&notify);
	notify.playerid = attack->get_uuid();

	if (a_owner)
	{
		notify.owneriid = a_owner->get_uuid();
		notify.ownername = a_owner->get_name();
	}
	else
	{
		notify.owneriid = attack->get_uuid();
	}
	
	notify.skillid = skill_id;
	notify.target_player = cached_hit_effect_point[0];
	notify.attack_pos = &attack_pos;
	notify.attack_cur_hp = attack->get_attr(PLAYER_ATTR_HP);
	notify.life_steal = life_steal;
	notify.damage_return = damage_return;

	std::vector<unit_struct *> both;
	both.push_back(attack);
	both.push_back(target);
	unit_struct::broadcast_to_many_sight(MSG_ID_SKILL_HIT_IMMEDIATE_NOTIFY, &notify, (pack_func)skill_hit_immediate_notify__pack, both);
//	player->broadcast_to_sight(MSG_ID_SKILL_HIT_IMMEDIATE_NOTIFY, &notify, (pack_func)skill_hit_immediate_notify__pack, true);

	if (!target->is_alive())
	{
		target->on_dead(a);
	}
	else
	{
		target->on_beattack(attack, skill_id, damage);
	}
	if (!attack->is_alive())
	{
		attack->on_dead(target);
	}	
}
/*
void monster_cast_delay_skill_to_player(uint64_t skill_id, monster_struct *monster, unit_struct *player)
{
	struct position *pos = monster->get_pos();
	monster->data->skill_id = skill_id;
	SkillCastDelayNotify notify;
	skill_cast_delay_notify__init(&notify);
	PosData cur_pos;
	pos_data__init(&cur_pos);
	cur_pos.pos_x = pos->pos_x;
	cur_pos.pos_z = pos->pos_z;	
	notify.cur_pos = &cur_pos;
	notify.playerid = monster->data->player_id;
	notify.skillid = skill_id;
	notify.targetid = player->get_uuid();
	
	player->broadcast_to_sight(MSG_ID_SKILL_CAST_DELAY_NOTIFY, &notify, (pack_func)skill_cast_delay_notify__pack, true);
}
*/
bool check_monster_relive(monster_struct *monster)
{
	if (time_helper::get_cached_time() > monster->data->relive_time)
		return true;
	return false;
}

//定点伤害范围的怪物
void monster_cast_skill_to_direct(uint64_t skill_id, monster_struct *monster, float direct_x, float direct_z, struct position *target)
{
	monster_try_skill_talk(monster, skill_id);
	
	SkillCastNotify notify;
	skill_cast_notify__init(&notify);
	notify.skillid = skill_id;
	notify.playerid = monster->data->player_id;
	PosData cur_pos;
	pos_data__init(&cur_pos);
	struct position *pos = monster->get_pos();
	cur_pos.pos_x = pos->pos_x;
	cur_pos.pos_z = pos->pos_z;
	notify.cur_pos = &cur_pos;
	notify.direct_x = direct_x;
	notify.direct_z = direct_z;

	if (target)
	{
		PosData target_pos;
		pos_data__init(&target_pos);
		target_pos.pos_x = target->pos_x;
		target_pos.pos_z = target->pos_z;			
		notify.target_pos = &target_pos;
	}
	
	monster->broadcast_to_sight(MSG_ID_SKILL_CAST_NOTIFY, &notify, (pack_func)skill_cast_notify__pack, false);
}

// void monster_cast_skill_to_pos(uint64_t skill_id, monster_struct *monster, float pos_x, float pos_z)
// {
// 	struct position *cur_pos = monster->get_pos();
// 	float direct_x = pos_x - cur_pos->pos_x;
// 	float direct_z = pos_z - cur_pos->pos_z;
// 	monster_cast_skill_to_direct(skill_id, monster, direct_x, direct_z, NULL);
// }

int get_monster_hp_percent(monster_struct *monster)
{
	double max_hp = monster->get_attr(PLAYER_ATTR_MAXHP);
	double cur_hp = monster->get_attr(PLAYER_ATTR_HP);
	return (int)(cur_hp / max_hp * 100);
}


// static void try_attack_target(monster_struct *monster, struct SkillTable *config)
// {
// 	struct position *my_pos = monster->get_pos();
// 	struct position *his_pos = monster->target->get_pos();

// 	if (monster->target && monster->target->is_too_high_to_beattack())
// 		return;

// 	assert(config && config->SkillType == 2);

// 	if (check_distance_in_range(my_pos, his_pos, config->SkillRange))
// 	{
// 		hit_notify_to_target(config->ID, monster, monster->target);
// 	}
// }

void try_attack_target(unit_struct *attack, unit_struct *target, struct SkillTable *config)
{
 	if (!target || !target->is_avaliable() || !target->is_alive())
		return;
	
	struct position *my_pos = attack->get_pos();
	struct position *his_pos = target->get_pos();

	if (target->is_too_high_to_beattack())
		return;

	assert(config && config->SkillType == 2);

	if (check_distance_in_range(my_pos, his_pos, config->SkillRange))
	{
		hit_notify_to_target(config->ID, attack, target);
	}
}

void try_attack(unit_struct *attack, struct SkillTable *config)
{
	assert(config && config->SkillType == 2);
	
	std::vector<unit_struct *> target;
	if (attack->count_skill_hit_unit(&target, config, false) != 0)
		return;
	hit_notify_to_many_target(config->ID, attack, &target);
}

void try_attack_friend(unit_struct *attack, struct SkillTable *config)
{
	assert(config && config->SkillType == 2);
	
	std::vector<unit_struct *> target;
	target.push_back(attack);
	if (attack->count_skill_hit_unit(&target, config, true) != 0)
		return;
	hit_notify_to_many_friend(config->ID, attack, &target);
}

void do_normal_patrol(monster_struct *monster)
{
	if (!monster->config)
		return;
//	if (monster->ai_type != AI_TYPE_NORMAL)
//		return;
	
	struct position *cur_pos = monster->get_pos();
	
	if (monster->create_config &&
		(fabsf(cur_pos->pos_x - monster->get_born_pos_x()) > (int)(monster->ai_config->GuardRange)
			|| fabsf(cur_pos->pos_z - monster->get_born_pos_z()) > (int)(monster->ai_config->GuardRange)))
	{
		return monster->go_back();
	}
	
	if (monster->is_unit_in_move())
		return;

	if (monster->try_active_attack())
	{
		monster->ai_state = AI_PURSUE_STATE;
		struct position *target_pos = monster->target->get_pos();
		monster->data->target_pos.pos_x = target_pos->pos_x;
		monster->data->target_pos.pos_z = target_pos->pos_z;		
//	uint64_t now = time_helper::get_cached_time();			
//	monster->reset_timer(now + 500);
		return;
	}
	
//	monster->data->move_path.pos[0].pos_x = monster->data->move_path.pos[monster->data->move_path.cur_pos].pos_x;
//	monster->data->move_path.pos[0].pos_z = monster->data->move_path.pos[monster->data->move_path.cur_pos].pos_z;
	monster->reset_pos();
	if (find_rand_position(monster->scene, &monster->data->move_path.pos[0],
			monster->ai_config->GuardRange, &monster->data->move_path.pos[1]) == 0)
	{
		monster->send_patrol_move();		
	}
}

void normal_ai_dead(monster_struct *monster, scene_struct *scene)
{
	monster->ai_state = AI_DEAD_STATE;
}

void do_normal_attack(monster_struct *monster)
{
	if (!monster->data)
		return;
//	if (monster->ai_type != AI_TYPE_NORMAL)
//		return;

//	LOG_DEBUG("%s: jack111: target = %p, skill[%u] idx[%u]", __FUNCTION__, monster->target, monster->data->skill_id, monster->data->skill_next_time_idx);
	
	if (!monster->target || !monster->target->is_avaliable())
	{
		monster->ai_state = AI_PATROL_STATE;
		return;
	}

	struct SkillTable *config = get_config_by_id(monster->data->skill_id, &skill_config);

	if (!config)
		return;

		//加血类技能
	if (config->TargetType[0] != 1)	
	{
		// if (config->MaxCount <= 1)
		// {
		// 	try_attack_target(monster, config);
		// }
		// else
		{
			try_attack_friend(monster, config);
		}

		
		monster->data->skill_id = 0;
		monster->ai_state = AI_PURSUE_STATE;		
			//计算硬直时间
		monster->data->ontick_time += count_skill_delay_time(config);
		return;
	}
	
//	struct ActiveSkillTable *act_config = get_config_by_id(config->SkillAffectId, &active_skill_config);
//	if (!act_config)
//		return;

//	LOG_DEBUG("%s: skill_id[%u], active_id[%lu] interval[%lu]", __FUNCTION__, monster->data->skill_id, config->SkillAffectId, act_config->Interval);
	uint64_t now = time_helper::get_cached_time();
	monster->reset_pos();
	
	if (monster->data->skill_next_time_idx < config->n_time_config && config->time_config[monster->data->skill_next_time_idx]->Interval > 0)
	{
		monster->data->ontick_time = now + config->time_config[monster->data->skill_next_time_idx]->Interval;

		// 	//第一次计算伤害
		// if (monster->data->skill_finished_time == 0)
		// {
		// 	monster->data->skill_finished_time = now + act_config->TotalSkillDelay;// - act_config->ActionTime;
		// }		
			//总时间到了，结束伤害
		if (monster->data->ontick_time > monster->data->skill_finished_time[monster->data->skill_next_time_idx])
		{
//			LOG_DEBUG("%s: jack111 skill[%u] 总时间到了，结束伤害", __FUNCTION__, monster->data->skill_id);
			
			monster->data->ontick_time = monster->data->skill_finished_time[monster->data->skill_next_time_idx] + random() % 1000;					
			monster->data->skill_finished_time[monster->data->skill_next_time_idx] = 0;
			monster->ai_state = AI_PURSUE_STATE;
			monster->data->target_pos.pos_x = 0;
			monster->data->target_pos.pos_z = 0;
			monster->data->skill_id = 0;
//			return;
		}
	}
	else
	{
//		LOG_DEBUG("%s: jack111 skill[%u] 总时间到了，结束伤害", __FUNCTION__, monster->data->skill_id);
		
		monster->data->ontick_time += config->TotalSkillDelay;
		monster->ai_state = AI_PURSUE_STATE;
		monster->data->target_pos.pos_x = 0;
		monster->data->target_pos.pos_z = 0;		
		monster->data->skill_id = 0;
	}

	SkillLvTable *lv_config = get_config_by_id(config->SkillLv, &skill_lv_config);	
		//召唤技能
	if (lv_config && lv_config->MonsterID != 0 && lv_config->MonsterLv != 0)
	{
		struct position *t_pos = monster->get_pos();
		struct position pos;

		if (get_circle_random_position_v3(monster->scene, t_pos, CALL_SKILL_RADIUS, &pos))
		{
			monster_struct *t_monster = monster_manager::create_monster_at_pos(monster->scene, lv_config->MonsterID,
			 	lv_config->MonsterLv, pos.pos_x, pos.pos_z, lv_config->MonsterEff, monster, 0);
			if (!t_monster)
			{
				LOG_ERR("%s: create monster at scene[%u] pos[%.1f][%.1f] failed", __FUNCTION__, monster->scene->m_id, pos.pos_x, pos.pos_z);
			}
			// if (t_monster)
			// {
			// 	assert(t_monster->ai_type == 15);
			// 	t_monster->ai_data.add_boss_hp_ai.boss_uuid = monster->get_uuid();
			// }
		}
		return;
	}

	if (config->MaxCount <= 1)
	{
		try_attack_target(monster, monster->target, config);
	}
	else
	{
		try_attack(monster, config);
	}
}

void do_normal_dead(monster_struct *monster)
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

void set_monster_skill_next_timeout(monster_struct *monster)
{
	uint64_t min = UINT64_MAX;
	for (int i = 0; i < MAX_SKILL_TIME_CONFIG_NUM; ++i)
	{
		if (monster->data->skill_next_time[i] != 0 && monster->data->skill_next_time[i] < min)
		{
			min = monster->data->skill_next_time[i];
			monster->data->skill_next_time_idx = i;
		}
	}
	if (min != UINT64_MAX)
		monster->data->ontick_time = min;	
}

void do_normal_pursue(monster_struct *monster)
{
	if (!monster->data)
		return;
//	if (monster->ai_type != AI_TYPE_NORMAL)
//		return;

	monster->on_pursue();
			
	if (!monster->target || !monster->target->is_avaliable()
		|| !monster->target->is_alive())
	{
		monster->ai_state = AI_PATROL_STATE;
		monster->go_back();
		return;
	}
	struct position *my_pos = monster->get_pos();
	struct position *his_pos = monster->target->get_pos();

		//如果目标的位置没有改变，就继续移动
	if (monster->is_unit_in_move())
	{
		if (monster->data->target_pos.pos_x == his_pos->pos_x
			&& monster->data->target_pos.pos_z == his_pos->pos_z)
		{
			monster->data->ontick_time += random() % 1000;		
			return;
		}
	}

	if (monster->ai && monster->ai->on_monster_ai_check_goback)
	{
		if (monster->ai->on_monster_ai_check_goback(monster))
			return monster->go_back();
	}
	else if (//monster->create_config &&
		(fabsf(my_pos->pos_x - monster->get_born_pos_x()) > (int)(monster->ai_config->ChaseRange)
			|| fabsf(my_pos->pos_z - monster->get_born_pos_z()) > (int)(monster->ai_config->ChaseRange)))
	{
		return monster->go_back();
	}

//	uint32_t skill_id = choose_rand_skill(monster);
	uint32_t skill_id = choose_skill_and_add_cd(monster);
	if (skill_id == 0)
		return;
	struct SkillTable *config = get_config_by_id(skill_id, &skill_config);
	if (config == NULL)
	{
		return;
	}

		//加血类技能
	if (config->TargetType[0] != 1)
	{
		uint64_t now = time_helper::get_cached_time();
		for (uint32_t i = 0; i < config->n_time_config; ++i)
		{
			if (config->time_config[i]->ActionTime > 0)
			{
				monster->data->skill_next_time[i] = now + config->time_config[i]->ActionTime;// + 1500;
				monster->data->skill_finished_time[i] = now + config->time_config[i]->ActionTime +
					config->time_config[i]->Frequency * config->time_config[i]->Interval;
			}
			else
			{
				monster->data->skill_next_time[i] = 0;
				LOG_ERR("%s: SKILL CONFIG ERR [%lu %lu]", __FUNCTION__, config->ID, config->time_config[i]->ID);
			}
		}

		set_monster_skill_next_timeout(monster);
		monster->ai_state = AI_ATTACK_STATE;
		monster->reset_pos();
		monster_cast_skill_to_target(skill_id, monster, monster->target, false);
		
// 		struct ActiveSkillTable *act_config = get_config_by_id(config->SkillAffectId, &active_skill_config);
// 		if (!act_config)
// 			return;

// 		if (act_config->ActionTime > 0)
// 		{
// 			uint64_t now = time_helper::get_cached_time();		
// 			monster->data->ontick_time = now + act_config->ActionTime;// + 1500;
// //			monster->data->skill_id = skill_id;
// 			monster->ai_state = AI_ATTACK_STATE;

// 			monster->reset_pos();
// 			monster_cast_skill_to_target(skill_id, monster, monster, false);		
// 			return;
// 		}
		return;
	}

//	LOG_DEBUG("%s: choose skill %u", __FUNCTION__, skill_id)

	if (!check_distance_in_range(my_pos, his_pos, config->SkillRange/*monster->ai_config->ActiveAttackRange*/))
	{
			//追击
		if (monster->ai_config->ChaseRange == 0)
//		if (!monster->ai_config->IsChase || monster->ai_config->ChaseRange == 0)
		{
				//不追击那么直接返回巡逻状态
			monster->data->skill_id = 0;			
			monster->ai_state = AI_PATROL_STATE;
			return;
		}
		
//		if (monster->is_unit_in_move())
//			return;
		monster->reset_pos();

//		int direct = getdirection(his_pos, my_pos);
//		if (get_circle_random_position(monster->scene, direct, my_pos, his_pos, /*monster->ai_config->ActiveAttackRange*/config->SkillRange * 5, &monster->data->move_path.pos[1]))
		if (get_circle_random_position_v2(monster->scene, my_pos, his_pos, /*monster->ai_config->ActiveAttackRange*/config->SkillRange, &monster->data->move_path.pos[1]))		
		{
			monster->send_patrol_move();
			monster->data->target_pos.pos_x = his_pos->pos_x;
			monster->data->target_pos.pos_z = his_pos->pos_z;				
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

	if (monster->target && monster->target->is_too_high_to_beattack())
		return;

		//主动技能
	if (config->SkillType == 2)
	{
		bool immediate_skill = true;
		uint64_t now = time_helper::get_cached_time();
		for (uint32_t i = 0; i < config->n_time_config; ++i)
		{
			if (config->time_config[i]->ActionTime > 0)
			{
				monster->data->skill_next_time[i] = now + config->time_config[i]->ActionTime;// + 1500;
//				monster->data->ontick_time = now + config->time_config[i]->ActionTime;// + 1500;
				monster->data->skill_finished_time[i] = now + config->time_config[i]->ActionTime +
					config->time_config[i]->Frequency * config->time_config[i]->Interval;

//				LOG_DEBUG("%s: jack111 skill[%u] 开始攻击，actiontime[%lu] finishedtime[%lu] frequency[%lu] interval[%lu]",
//					__FUNCTION__, config->ID, config->time_config[i]->ActionTime, monster->data->skill_finished_time[i],
//					config->time_config[i]->Frequency, config->time_config[i]->Interval);
				
				immediate_skill = false;
			}
			else
			{
				monster->data->skill_next_time[i] = 0;
				LOG_ERR("%s: SKILL CONFIG ERR [%lu %lu]", __FUNCTION__, config->ID, config->time_config[i]->ID);
			}
		}
		
		if (!immediate_skill)
		{
			set_monster_skill_next_timeout(monster);
			monster->ai_state = AI_ATTACK_STATE;

			monster->reset_pos();
			if (config->RangeType == 4 || config->RangeType == 3)
				monster_cast_skill_to_target(skill_id, monster, monster->target, true);
			else
				monster_cast_skill_to_target(skill_id, monster, monster->target, false);
			return;
		}
		
// 		struct ActiveSkillTable *act_config = get_config_by_id(config->SkillAffectId, &active_skill_config);
// 		if (!act_config)
// 			return;

// 		if (act_config->ActionTime > 0)
// 		{
// 			uint64_t now = time_helper::get_cached_time();		
// 			monster->data->ontick_time = now + act_config->ActionTime;// + 1500;
// 			monster->data->skill_finished_time = now + act_config->TotalSkillDelay + act_config->ActionTime;

// //			monster->data->skill_id = skill_id;
// //			monster->data->angle = -(pos_to_angle(his_pos->pos_x - my_pos->pos_x, his_pos->pos_z - my_pos->pos_z));
// 			monster->ai_state = AI_ATTACK_STATE;

// 			monster->reset_pos();
// 			if (config->RangeType == 4 || config->RangeType == 3)
// 				monster_cast_skill_to_target(skill_id, monster, monster->target, true);
// 			else
// 				monster_cast_skill_to_target(skill_id, monster, monster->target, false);		
// 			return;
// 		}
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
