#include <math.h>
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

static void try_attack_target(monster_struct *monster, struct SkillTable *config);
static void try_attack(monster_struct *monster, struct SkillTable *config);

uint32_t count_skill_delay_time(struct SkillTable *config)
{
	int ret = 0;
	if (config == NULL)
		return (0);
		//普通攻击或者主动攻击, 计算硬直时间
	if (config->SkillType == 1 || config->SkillType == 2)
	{
		struct ActiveSkillTable *act_config = get_config_by_id(config->SkillAffectId, &active_skill_config);
		if (act_config)
		{
			ret += act_config->TotalSkillDelay + random() % (500 * act_config->n_SkillLength);
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

	if (!monster->is_skill_in_cd(0, now))
	{
		monster->add_skill_cd(0, now);
		return monster->config->Skill[0];				
	}
	
	return (0);
}

uint32_t choose_first_skill(monster_struct *monster)
{
	assert(monster->config->n_Skill != 0);
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
void monster_hit_notify_to_many_player(uint64_t skill_id, monster_struct *monster, player_struct *owner, std::vector<unit_struct *> *target)
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
	struct SkillLvTable *lv_config1, *lv_config2;
	struct PassiveSkillTable *pas_config;
	struct SkillTable *ski_config;
	get_skill_configs(1, skill_id, &ski_config, &lv_config1, &pas_config, &lv_config2, NULL);
	if (!lv_config1 && !lv_config2)
	{
		LOG_ERR("%s %d: skill[%lu] no config", __FUNCTION__, __LINE__, skill_id);
		return;
	}

	uint32_t life_steal = 0;
	uint32_t damage_return = 0;

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
			if (get_unit_fight_type(monster, player) != UNIT_FIGHT_TYPE_ENEMY)								
				continue;			
		}
		
		if (!player->is_alive())
		{
			continue;
		}

		if (player->is_too_high_to_beattack())
			return;		
		
		cached_hit_effect_point[n_hit_effect] = &cached_hit_effect[n_hit_effect];
		skill_hit_effect__init(&cached_hit_effect[n_hit_effect]);
		uint32_t add_num = 0;
		int32_t damage = 0;
		int32_t other_rate = count_other_skill_damage_effect(monster, player);						
		damage += count_skill_total_damage(UNIT_FIGHT_TYPE_ENEMY, ski_config, lv_config1,
			pas_config, lv_config2,
			monster, player,
			&cached_hit_effect[n_hit_effect].effect,
			&cached_buff_id[n_buff],
			&cached_buff_end_time[n_buff],
			&add_num, other_rate);

		life_steal += monster->count_life_steal_effect(damage);
		damage_return += monster->count_damage_return(damage, player);

		if (player->get_unit_type() == UNIT_TYPE_PLAYER)
		{
			player_struct *t = ((player_struct *)player);
			raid_struct *raid = t->get_raid();
			if (raid)
				raid->on_monster_attack(monster, t, damage);

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
			player->on_beattack(monster, skill_id, damage);						
		}
		else
		{
			if (monster && monster->data && monster->data->owner)
			{
				player_struct *p = player_manager::get_player_by_id(monster->data->owner);
				if (p)
					player->on_dead(p);
			}
			else
			{
				player->on_dead(monster);
			}
		}

	}

	target->push_back(monster);

	SkillHitNotify notify;
	skill_hit_notify__init(&notify);
	notify.playerid = monster->data->player_id;

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

	notify.attack_cur_hp = monster->get_attr(PLAYER_ATTR_HP);
	notify.life_steal = life_steal;
	notify.damage_return = damage_return;

	PosData attack_pos;
	pos_data__init(&attack_pos);
	attack_pos.pos_x = monster->get_pos()->pos_x;
	attack_pos.pos_z = monster->get_pos()->pos_z;
	notify.attack_pos = &attack_pos;
	player_struct::broadcast_to_many_sight(MSG_ID_SKILL_HIT_NOTIFY, &notify, (pack_func)skill_hit_notify__pack, *target);
	
	if (!monster->is_alive())
	{
		monster->on_dead(monster);
	}
}

void monster_hit_notify_to_player(uint64_t skill_id, monster_struct *monster, unit_struct *player)
{
	std::vector<unit_struct *> target;
	target.push_back(player);
	monster_hit_notify_to_many_player(skill_id, monster, NULL, &target);
}

void monster_cast_call_monster_skill(monster_struct *monster, uint64_t skill_id)
{
	SkillTable *config = get_config_by_id(skill_id, &skill_config);
	if (config && config->IsMonster)
	{
		monster_cast_skill_to_player(monster->ai_data.leixinye_ai.call_skill_id, monster, NULL, false);
		SkillLvTable *lv_config = get_config_by_id(config->SkillLv, &skill_lv_config);
		if (lv_config && lv_config->MonsterID != 0 && lv_config->MonsterLv != 0)
		{
			struct position *t_pos = monster->get_pos();
			monster_manager::create_monster_at_pos(monster->scene, lv_config->MonsterID,
				lv_config->MonsterLv, t_pos->pos_x, t_pos->pos_z, lv_config->MonsterEff, monster, 0);
		}
	}	
}

static void monster_try_skill_talk(monster_struct *monster, uint64_t skill_id)
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

void monster_cast_skill_to_player(uint64_t skill_id, monster_struct *monster, unit_struct *player, bool use_target_pos)
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

	PosData target_pos;

	if (player)
	{
		struct position *player_pos = player->get_pos();
		notify.direct_x = player_pos->pos_x - pos->pos_x;
		notify.direct_z = player_pos->pos_z - pos->pos_z;

		if (use_target_pos)
		{
			pos_data__init(&target_pos);
			target_pos.pos_x = player_pos->pos_x;
			target_pos.pos_z = player_pos->pos_z;			
			notify.target_pos = &target_pos;
		}
	}
	monster->broadcast_to_sight(MSG_ID_SKILL_CAST_NOTIFY, &notify, (pack_func)skill_cast_notify__pack, false);
}

void monster_cast_skill_to_friend(monster_struct *monster, struct SkillTable *config)
{
	unit_struct *target = NULL;
	monster_try_skill_talk(monster, config->ID);
	for (uint32_t i = 0; !target && i < config->n_TargetType; ++i)
	{
		switch (config->TargetType[i])
		{
			case 101://自身
			default:
				target = monster;
				break;
		}
	}
	
	if (!target)
		return;
	SkillCastNotify notify;
	skill_cast_notify__init(&notify);
	notify.skillid = config->ID;
	notify.playerid = monster->data->player_id;
	PosData cur_pos;
	pos_data__init(&cur_pos);
	struct position *pos = monster->get_pos();
	cur_pos.pos_x = pos->pos_x;
	cur_pos.pos_z = pos->pos_z;		
	notify.cur_pos = &cur_pos;

//	PosData target_pos;

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
	monster->broadcast_to_sight(MSG_ID_SKILL_CAST_NOTIFY, &notify, (pack_func)skill_cast_notify__pack, false);

	if (config->MaxCount <= 1)
	{
		try_attack_target(monster, config);
	}
	else
	{
		try_attack(monster, config);
	}
}

void monster_cast_immediate_skill_to_player(uint64_t skill_id, monster_struct *monster, player_struct *owner, unit_struct *player)
{
	monster_try_skill_talk(monster, skill_id);	
//	if (!check_can_attack(monster, player))

	if (owner)
	{
		if (get_unit_fight_type(owner, player) != UNIT_FIGHT_TYPE_ENEMY)
			return;
	}
	else
	{
		if (get_unit_fight_type(monster, player) != UNIT_FIGHT_TYPE_ENEMY)			
			return;
	}

//	if (player->buff_state & BUFF_STATE_GOD)
//	{
//		return;
//	}
	
	int n_hit_effect = 0;
	int n_buff = 0;
	assert(player && player->is_avaliable());
	cached_hit_effect_point[n_hit_effect] = &cached_hit_effect[n_hit_effect];
	skill_hit_effect__init(&cached_hit_effect[n_hit_effect]);
	uint32_t add_num = 0;
	int32_t damage = 0;

	struct SkillLvTable *lv_config1, *lv_config2;
	struct PassiveSkillTable *pas_config;
	struct SkillTable *ski_config;
	get_skill_configs(1, skill_id, &ski_config, &lv_config1, &pas_config, &lv_config2, NULL);
	if (!lv_config1 && !lv_config2)
	{
		LOG_ERR("%s %d: skill[%lu] no config", __FUNCTION__, __LINE__, skill_id);
		return;
	}
	
	int32_t other_rate = count_other_skill_damage_effect(monster, player);					
	damage += count_skill_total_damage(UNIT_FIGHT_TYPE_ENEMY, ski_config, lv_config1,
		pas_config, lv_config2,
		monster, player,
		&cached_hit_effect[n_hit_effect].effect,
		&cached_buff_id[n_buff],
		&cached_buff_end_time[n_buff],
		&add_num, other_rate);

	player->on_hp_changed(damage);

	if (player->get_unit_type() == UNIT_TYPE_PLAYER)
	{
		player_struct *t = ((player_struct *)player);
		raid_struct *raid = t->get_raid();
		if (raid)
			raid->on_monster_attack(monster, t, damage);
	}

	LOG_DEBUG("%s: unit[%lu][%p] attack unit[%lu][%p] damage[%d] hp[%f]",
		__FUNCTION__, monster->get_uuid(), monster,
		player->get_uuid(), player, damage, player->get_attr(PLAYER_ATTR_HP));

	uint32_t life_steal = monster->count_life_steal_effect(damage);
	uint32_t damage_return = monster->count_damage_return(damage, player);

	monster->on_hp_changed(damage_return);

	PosData attack_pos;
	pos_data__init(&attack_pos);
	attack_pos.pos_x = monster->get_pos()->pos_x;
	attack_pos.pos_z = monster->get_pos()->pos_z;
	
	cached_hit_effect[n_hit_effect].playerid = player->get_uuid();
	cached_hit_effect[n_hit_effect].n_add_buff = add_num;
	cached_hit_effect[n_hit_effect].add_buff = &cached_buff_id[n_buff];
//	cached_hit_effect[n_hit_effect].add_buff_end_time = &cached_buff_end_time[n_buff];	
	cached_hit_effect[n_hit_effect].hp_delta = damage;
	cached_hit_effect[n_hit_effect].cur_hp = player->get_attr(PLAYER_ATTR_HP);
//	cached_hit_effect.attack_pos = &attack_pos;
	cached_hit_effect[n_hit_effect].target_pos = &cached_target_pos[n_hit_effect];	
//	pos_data__init(&cached_attack_pos[n_hit_effect]);
	pos_data__init(&cached_target_pos[n_hit_effect]);	
//	cached_attack_pos[n_hit_effect].pos_x = monster->get_pos()->pos_x;
//	cached_attack_pos[n_hit_effect].pos_z = monster->get_pos()->pos_z;
	cached_target_pos[n_hit_effect].pos_x = player->get_pos()->pos_x;
	cached_target_pos[n_hit_effect].pos_z = player->get_pos()->pos_z;			
	
	n_buff += add_num;
	++n_hit_effect;

	SkillHitImmediateNotify notify;
	skill_hit_immediate_notify__init(&notify);
	notify.playerid = monster->data->player_id;

	if (owner)
	{
		notify.owneriid = monster->data->owner;
		notify.ownername = owner->get_name();
	}
	else
	{
		notify.owneriid = monster->get_uuid();
	}
	
	notify.skillid = skill_id;
	notify.target_player = cached_hit_effect_point[0];
	notify.attack_pos = &attack_pos;
	notify.attack_cur_hp = monster->get_attr(PLAYER_ATTR_HP);
	notify.life_steal = life_steal;
	notify.damage_return = damage_return;

	std::vector<unit_struct *> both;
	both.push_back(player);
	both.push_back(monster);
	unit_struct::broadcast_to_many_sight(MSG_ID_SKILL_HIT_IMMEDIATE_NOTIFY, &notify, (pack_func)skill_hit_immediate_notify__pack, both);
//	player->broadcast_to_sight(MSG_ID_SKILL_HIT_IMMEDIATE_NOTIFY, &notify, (pack_func)skill_hit_immediate_notify__pack, true);

	if (!player->is_alive())
	{
		player->on_dead(monster);
	}
	else
	{
		player->on_beattack(monster, skill_id, damage);
	}
	if (!monster->is_alive())
	{
		monster->on_dead(player);
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
void monster_cast_skill_to_direct(uint64_t skill_id, monster_struct *monster, float direct_x, float direct_z)
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
	monster->broadcast_to_sight(MSG_ID_SKILL_CAST_NOTIFY, &notify, (pack_func)skill_cast_notify__pack, true);
}

void monster_cast_skill_to_pos(uint64_t skill_id, monster_struct *monster, float pos_x, float pos_z)
{
	struct position *cur_pos = monster->get_pos();
	float direct_x = pos_x - cur_pos->pos_x;
	float direct_z = pos_z - cur_pos->pos_z;
	monster_cast_skill_to_direct(skill_id, monster, direct_x, direct_z);
}

int get_monster_hp_percent(monster_struct *monster)
{
	double max_hp = monster->get_attr(PLAYER_ATTR_MAXHP);
	double cur_hp = monster->get_attr(PLAYER_ATTR_HP);
	return (int)(cur_hp / max_hp * 100);
}


static void try_attack_target(monster_struct *monster, struct SkillTable *config)
{
	struct position *my_pos = monster->get_pos();
	struct position *his_pos = monster->target->get_pos();

	if (monster->target && monster->target->is_too_high_to_beattack())
		return;

	assert(config && config->SkillType == 2);
//	LOG_DEBUG("%s monster hit skill %u", __FUNCTION__, config->ID);

	if (check_distance_in_range(my_pos, his_pos, config->SkillRange/*monster->ai_config->ActiveAttackRange*/))
	{
		monster_hit_notify_to_player(config->ID, monster, monster->target);
	}
}

static void try_attack(monster_struct *monster, struct SkillTable *config)
{
	assert(config && config->SkillType == 2);
	
	std::vector<unit_struct *> target;
	if (monster->count_skill_hit_unit(&target, config, monster->target) != 0)
		return;
	monster_hit_notify_to_many_player(config->ID, monster, NULL, &target);
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
	if (!monster->target || !monster->target->is_avaliable())
	{
		monster->ai_state = AI_PATROL_STATE;
		return;
	}

	struct SkillTable *config = get_config_by_id(monster->data->skill_id, &skill_config);
	
	if (!config)
		return;
	struct ActiveSkillTable *act_config = get_config_by_id(config->SkillAffectId, &active_skill_config);
	if (!act_config)
		return;

	LOG_DEBUG("%s: skill_id[%u], active_id[%lu] interval[%lu]", __FUNCTION__, monster->data->skill_id, config->SkillAffectId, act_config->Interval);
	uint64_t now = time_helper::get_cached_time();
	monster->reset_pos();
	if (act_config->Interval > 0)
	{
		monster->data->ontick_time = now + act_config->Interval;

			//第一次计算伤害
		if (monster->data->skill_finished_time == 0)
		{
			monster->data->skill_finished_time = now + act_config->TotalSkillDelay - act_config->ActionTime;
		}
			//总时间到了，结束伤害
		else if (now > monster->data->skill_finished_time)
		{
			monster->data->skill_finished_time = 0;
			monster->ai_state = AI_PURSUE_STATE;
			monster->data->target_pos.pos_x = 0;
			monster->data->target_pos.pos_z = 0;
			monster->data->skill_id = 0;
			return;
		}
	}
	else
	{
		monster->data->ontick_time += act_config->TotalSkillDelay;
		monster->ai_state = AI_PURSUE_STATE;
		monster->data->target_pos.pos_x = 0;
		monster->data->target_pos.pos_z = 0;		
		monster->data->skill_id = 0;
	}

		//召唤技能
	if (config->IsMonster)
	{
		SkillLvTable *lv_config = get_config_by_id(config->SkillLv, &skill_lv_config);
		if (!lv_config || lv_config->MonsterID == 0 || lv_config->MonsterLv == 0)
			return;
		
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
		try_attack_target(monster, config);
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
	// if (config->TargetType[0] != 1)
	// {
	// 	monster_cast_skill_to_friend(monster, config);
	// 	monster->data->skill_id = 0;
	// 		//计算硬直时间
	// 	monster->data->ontick_time += count_skill_delay_time(config);
	// 	return;
	// }

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
		struct ActiveSkillTable *act_config = get_config_by_id(config->SkillAffectId, &active_skill_config);
		if (!act_config)
			return;

		if (act_config->ActionTime > 0)
		{
			uint64_t now = time_helper::get_cached_time();		
			monster->data->ontick_time = now + act_config->ActionTime;// + 1500;
			monster->data->skill_id = skill_id;
			monster->data->angle = -(pos_to_angle(his_pos->pos_x - my_pos->pos_x, his_pos->pos_z - my_pos->pos_z));
//			LOG_DEBUG("jacktang: mypos[%.2f][%.2f] hispos[%.2f][%.2f]", my_pos->pos_x, my_pos->pos_z, his_pos->pos_x, his_pos->pos_z);
			monster->ai_state = AI_ATTACK_STATE;

			monster->reset_pos();
			monster_cast_skill_to_player(skill_id, monster, monster->target, false);		
			return;
		}
	}

	monster->reset_pos();
	monster_cast_immediate_skill_to_player(skill_id, monster, NULL, monster->target);
		//被反弹死了
	if (!monster->data)
		return;
	
	monster->data->skill_id = 0;

		//计算硬直时间
	monster->data->ontick_time += count_skill_delay_time(config);
}
