#include "pvp_player_ai.h"
#include "path_algorithm.h"
#include "player_manager.h"
#include "time_helper.h"
#include "msgid.h"
#include "camp_judge.h"
#include "buff_manager.h"
#include "pvp_match_manager.h"
#include "raid.h"
#include "check_range.h"
#include "count_skill_damage.h"
#include "camp_judge.h"
#include "cached_hit_effect.h"
#include "buff.h"
#include "skill.h"
#include "count_pvp_skill_unit.h"

static bool do_attack(player_struct *player, struct ai_player_data *ai_player_data, player_struct *target, uint32_t skill_id);
static player_struct **get_enemy_team_player(player_struct *player, struct ai_player_data **ai_player_data);
static void do_ai_player_attack(player_struct *monster, struct ai_player_data *ai_player_data, player_struct **enemy);
static void do_ai_player_relive(raid_struct *raid, player_struct *player, struct ai_player_data *ai_player_data, player_struct **enemy);

static void add_patrol_index(player_struct *player)
{
	player->data->patrol_index++;
	if (player->data->patrol_index >= player->ai_patrol_config->n_patrol)
		player->data->patrol_index = 0;
}
static void sub_patrol_index(player_struct *player)
{
	if (player->data->patrol_index == 0)
		player->data->patrol_index = player->ai_patrol_config->n_patrol - 1;
	else
		player->data->patrol_index--;
}

static void find_next_position(player_struct *player)
{
	add_patrol_index(player);
	player->data->move_path.pos[1].pos_x = player->ai_patrol_config->patrol[player->data->patrol_index]->pos_x;
	player->data->move_path.pos[1].pos_z = player->ai_patrol_config->patrol[player->data->patrol_index]->pos_z;	
}

static void reset_patrol_index(player_struct *player)
{
	uint64_t min_distance = UINT64_MAX;
	int min_index = 0;
	
	struct position *pos = player->get_pos();
	int max_index = player->ai_patrol_config->n_patrol;
	max_index = (max_index + 2) / 2;
	for (int i = 0; i < max_index; ++i)
	{
		double x = pos->pos_x - player->ai_patrol_config->patrol[i]->pos_x;
		double z = pos->pos_z - player->ai_patrol_config->patrol[i]->pos_z;		
		uint64_t distance = x * x + z * z;
		if (distance <= min_distance)
		{
			min_distance = distance;
			min_index = i;
		}
	}
	player->data->patrol_index = min_index;
	sub_patrol_index(player);
}

static uint32_t choose_rand_skill(player_struct *player)
{
	return player->m_skill.GetRandSkillId();
}

static void pvp_player_ai_send_move(player_struct *player)
{
	player->data->move_path.start_time = time_helper::get_cached_time();
	player->data->move_path.max_pos = 1;
	player->data->move_path.cur_pos = 0;

//	LOG_DEBUG("aitest: [%s] index = %u, move from[%.1f][%.1f] to [%.1f][%.1f]", player->get_name(), player->data->patrol_index,
//		player->data->move_path.pos[0].pos_x, player->data->move_path.pos[0].pos_z,
//		player->data->move_path.pos[1].pos_x, player->data->move_path.pos[1].pos_z);		

	MoveNotify notify;
	move_notify__init(&notify);
	notify.playerid = player->data->player_id;
	notify.data = player->pack_unit_move_path(&notify.n_data);

	player->broadcast_to_sight(MSG_ID_MOVE_NOTIFY, &notify, (pack_func)move_notify__pack, true);
	unit_struct::reset_pos_pool();
}

static void pvp_player_ai_tick(player_struct *player)
{
	if (player->buff_state & BUFF_STATE_STUN)
	{
		LOG_DEBUG("aitest: [%s] lock", player->get_name());		
		return;
	}
	
	if (player->data->stop_ai)
		return;
	if (player->is_unit_in_move())
		return;
	if (player->scene->get_scene_type() != SCENE_TYPE_RAID)
		return;

	player->reset_pos();

//	raid_struct *raid = (raid_struct *)player->scene;
//	assert(raid->m_config->DengeonRank == 5 || raid->m_config->DengeonRank == 6);

		// try attack others
	player_struct **enemy = NULL;
	struct ai_player_data *ai_player_data = NULL;
	enemy = get_enemy_team_player(player, &ai_player_data);
	assert(enemy);

	if (time_helper::get_cached_time() < ai_player_data->ontick_time)
		return;

	if (!player->is_alive())
	{
		return do_ai_player_relive((raid_struct *)player->scene, player, ai_player_data, enemy);
	}

	LOG_DEBUG("aitest: [%s]ai_state = %d", player->get_name(), ai_player_data->ai_state);


	if (ai_player_data->ai_state == AI_ATTACK_STATE)
		return do_ai_player_attack(player, ai_player_data, enemy);

	uint32_t skill_id = choose_rand_skill(player);

	if (skill_id != 0)
	{
		for (int i = 0; i < MAX_TEAM_MEM; ++i)
		{
			if (!enemy[i])
				continue;
			if (!enemy[i]->is_alive())
				continue;

			if (ai_player_data->target_player_id)
			{
				if (ai_player_data->target_player_id != enemy[i]->get_uuid())
					continue;
					//检查追击距离
				struct position *his_pos = enemy[i]->get_pos();
				struct position ori_pos;
				ori_pos.pos_x = player->ai_patrol_config->patrol[player->data->patrol_index]->pos_x;
				ori_pos.pos_z = player->ai_patrol_config->patrol[player->data->patrol_index]->pos_z;
				if (!check_distance_in_range(his_pos, &ori_pos, player->data->chase_range))
				{
					LOG_DEBUG("aitest: [%s] chase distance = %.1f %.1f", player->get_name(),
						his_pos->pos_x - ori_pos.pos_x, his_pos->pos_z - ori_pos.pos_z);
					break;
				}
			}
			else
			{
					//检查主动攻击距离
				struct position *my_pos = player->get_pos();
				struct position *his_pos = enemy[i]->get_pos();
				if (!check_distance_in_range(my_pos, his_pos, player->data->active_attack_range))
				{
//					LOG_DEBUG("aitest: [%s] active attack distance = %.1f %.1f", player->get_name(),
//						his_pos->pos_x - my_pos->pos_x, his_pos->pos_z - my_pos->pos_z);
					continue;
				}
			}
			
			if (do_attack(player, ai_player_data, enemy[i], skill_id))
			{
				ai_player_data->target_player_id = enemy[i]->get_uuid();
//				LOG_DEBUG("aitest: [%s] try attack %s", player->get_name(), enemy[i]->get_name());
				return;
			}
		}
	}

	LOG_DEBUG("aitest: [%s]rand move, index = %u, skill_id = %u", player->get_name(), player->data->patrol_index, skill_id);
	ai_player_data->target_player_id = 0;

//	if (find_rand_position(player->scene, &player->data->move_path.pos[0],
//			3, &player->data->move_path.pos[1]) == 0)
	{
		find_next_position(player);
		pvp_player_ai_send_move(player);
		// player->data->move_path.start_time = time_helper::get_cached_time();
		// player->data->move_path.max_pos = 1;
		// player->data->move_path.cur_pos = 0;

		// MoveNotify notify;
		// move_notify__init(&notify);
		// notify.playerid = player->data->player_id;
		// notify.data = player->pack_unit_move_path(&notify.n_data);

		// player->broadcast_to_sight(MSG_ID_MOVE_NOTIFY, &notify, (pack_func)move_notify__pack, true);
		// unit_struct::reset_pos_pool();
	}
	// else
	// {
	// 	LOG_DEBUG("aitest[%s]: rand move failed", player->get_name());		
	// }
}

static void ai_player_hit_notify_to_many_player(uint64_t skill_id, player_struct *monster, std::vector<unit_struct *> *target)
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
		UNIT_FIGHT_TYPE fight_type = get_unit_fight_type(monster, player);
		if (fight_type != UNIT_FIGHT_TYPE_ENEMY)						
		{
			continue;
		}

		if (!player->is_alive())
		{
			continue;
		}

		LOG_DEBUG("aitest: [%s]attack target %lu", monster->get_name(), player->get_uuid());

		cached_hit_effect_point[n_hit_effect] = &cached_hit_effect[n_hit_effect];
		skill_hit_effect__init(&cached_hit_effect[n_hit_effect]);
		uint32_t add_num = 0;
		int32_t damage = 0;
		int32_t other_rate = count_other_skill_damage_effect(monster, player);							
		damage += count_skill_total_damage(fight_type, ski_config, lv_config1,
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
				raid->on_player_attack(monster, t, damage);
		}

		LOG_DEBUG("aitest: [%s] unit[%lu][%p] damage[%d] hp[%f]", monster->get_name(), player->get_uuid(), player, damage, player->get_attr(PLAYER_ATTR_HP));

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
			player->on_dead(monster);
		}

	}

	target->push_back(monster);

	SkillHitNotify notify;
	skill_hit_notify__init(&notify);
	notify.playerid = monster->data->player_id;
	notify.owneriid = notify.playerid;
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

	if (!monster->is_alive())
	{
		monster->on_dead(monster);
	}
	player_struct::broadcast_to_many_sight(MSG_ID_SKILL_HIT_NOTIFY, &notify, (pack_func)skill_hit_notify__pack, *target);
}

__attribute_used__ static void ai_player_hit_notify_to_player(uint64_t skill_id, player_struct *monster, unit_struct *player)
{
	std::vector<unit_struct *> target;
	target.push_back(player);
	ai_player_hit_notify_to_many_player(skill_id, monster, &target);
}

static void ai_player_cast_skill_to_player(uint64_t skill_id, player_struct *monster, unit_struct *player)
{
	SkillCastNotify notify;
	skill_cast_notify__init(&notify);
	notify.skillid = skill_id;
	notify.playerid = monster->data->player_id;
	PosData cur_pos;
	pos_data__init(&cur_pos);
	struct position *pos = monster->get_pos();
	struct position *player_pos = player->get_pos();
	cur_pos.pos_x = pos->pos_x;
	cur_pos.pos_z = pos->pos_z;
	notify.cur_pos = &cur_pos;
	notify.direct_x = player_pos->pos_x - pos->pos_x;
	notify.direct_z = player_pos->pos_z - pos->pos_z;
	player->broadcast_to_sight(MSG_ID_SKILL_CAST_NOTIFY, &notify, (pack_func)skill_cast_notify__pack, true);
}

static void ai_player_cast_immediate_skill_to_player(uint64_t skill_id, player_struct *monster, unit_struct *player)
{
//	if (!check_can_attack(monster, player))
	UNIT_FIGHT_TYPE fight_type = get_unit_fight_type(monster, player);	
	if (fight_type != UNIT_FIGHT_TYPE_ENEMY)										
	{
		return;
	}
	if (!player->is_alive())
	{
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
	damage += count_skill_total_damage(fight_type, ski_config, lv_config1,
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
			raid->on_player_attack(monster, t, damage);
	}

	LOG_DEBUG("aitest: %s [%s] unit[%lu] damage[%d] hp[%f]", __FUNCTION__, monster->get_name(), player->get_uuid(), damage, player->get_attr(PLAYER_ATTR_HP));

	LOG_DEBUG("%s: unit[%lu][%p] damage[%d] hp[%f]", __FUNCTION__, player->get_uuid(), player, damage, player->get_attr(PLAYER_ATTR_HP));

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
	notify.owneriid = monster->data->player_id;	
	notify.skillid = skill_id;
	notify.target_player = cached_hit_effect_point[0];
	notify.attack_pos = &attack_pos;
	notify.attack_cur_hp = monster->get_attr(PLAYER_ATTR_HP);
	notify.life_steal = life_steal;
	notify.damage_return = damage_return;

	if (!player->is_alive())
	{
		player->on_dead(player);
	}
	else
	{
		player->on_beattack(monster, skill_id, damage);
	}
	if (!monster->is_alive())
	{
		monster->on_dead(player);
	}

	player->broadcast_to_sight(MSG_ID_SKILL_HIT_IMMEDIATE_NOTIFY, &notify, (pack_func)skill_hit_immediate_notify__pack, true);
}

static uint32_t count_skill_delay_time(struct SkillTable *config)
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

static bool do_attack(player_struct *player, struct ai_player_data *ai_player_data, player_struct *target, uint32_t skill_id)
{
	struct position *my_pos = player->get_pos();
	struct position *his_pos = target->get_pos();

	LOG_DEBUG("aitest: %s: [%s]skill_id = %u", __FUNCTION__, player->get_name(), skill_id);
	
	if (skill_id == 0)
		return false;

	struct SkillLvTable *lv_config1, *lv_config2;
	struct PassiveSkillTable *pas_config;
	struct SkillTable *config;
	get_skill_configs(1, skill_id, &config, &lv_config1, &pas_config, &lv_config2, NULL);
	if (!config || !lv_config1)
	{
		LOG_ERR("%s: %lu skill[%u] do not have config", __FUNCTION__, player->get_uuid(), skill_id);
		return false;
	}
	
	// struct SkillTable *config = get_config_by_id(skill_id, &skill_config);
	// if (config == NULL)
	// {
	// 	return false;
	// }

	skill_struct *skill_struct = player->m_skill.GetSkillStructFromFuwen(skill_id);
	if (!skill_struct)
	{
		LOG_ERR("%s: %lu can not find skill struct %u", __FUNCTION__, player->get_uuid(), skill_id);
		return false;
	}

//	LOG_DEBUG("%s: choose skill %u", __FUNCTION__, skill_id)

	if (!check_distance_in_range(my_pos, his_pos, config->SkillRange/*monster->ai_config->ActiveAttackRange*/))
	{
			// move to target
		if (get_circle_random_position_v2(player->scene, my_pos, his_pos, config->SkillRange, &player->data->move_path.pos[1]))
		{
			pvp_player_ai_send_move(player);
			return true;
		}
		LOG_DEBUG("aitest: %s %d: [%s]no target", __FUNCTION__, __LINE__, player->get_name());	
		return false;
	}


	LOG_DEBUG("aitest: %s %d: [%s]skill_type = %lu", __FUNCTION__, __LINE__, player->get_name(), config->SkillType);	

		//主动技能
	if (config->SkillType == 2)
	{
		struct ActiveSkillTable *act_config = get_config_by_id(config->SkillAffectId, &active_skill_config);
		if (!act_config)
			return false;
		if (act_config->ActionTime > 0)
		{
			uint64_t now = time_helper::get_cached_time();
			ai_player_data->ontick_time = now + act_config->ActionTime;// + 1500;
			ai_player_data->skill_id = skill_id;
			ai_player_data->angle = -(pos_to_angle(his_pos->pos_x - my_pos->pos_x, his_pos->pos_z - my_pos->pos_z));
// //			LOG_DEBUG("jacktang: mypos[%.2f][%.2f] hispos[%.2f][%.2f]", my_pos->pos_x, my_pos->pos_z, his_pos->pos_x, his_pos->pos_z);
			ai_player_data->ai_state = AI_ATTACK_STATE;

			player->reset_pos();
			ai_player_cast_skill_to_player(skill_id, player, target);

			skill_struct->add_cd(lv_config1, act_config);

			LOG_DEBUG("aitest: %s %d: [%s]onticktime = %lu", __FUNCTION__, __LINE__, player->get_name(), act_config->ActionTime);	
			return true;
		}
	}

	player->reset_pos();
	ai_player_cast_immediate_skill_to_player(skill_id, player, target);

		//被反弹死了
	if (!player->is_alive())
		return false;

	ai_player_data->skill_id = 0;

	//	//计算硬直时间
	uint32_t t = count_skill_delay_time(config);
	ai_player_data->ontick_time += t;

	LOG_DEBUG("aitest: %s %d: [%s]onticktime += %u", __FUNCTION__, __LINE__, player->get_name(), t);		
	return true;
}

__attribute_used__ static void try_attack_target(player_struct *monster, player_struct *target, struct SkillTable *config)
{
	struct position *my_pos = monster->get_pos();
	struct position *his_pos = target->get_pos();

	assert(config && config->SkillType == 2);
//	LOG_DEBUG("%s monster hit skill %u", __FUNCTION__, config->ID);
	if (check_distance_in_range(my_pos, his_pos, config->SkillRange/*monster->ai_config->ActiveAttackRange*/))
	{
		ai_player_hit_notify_to_player(config->ID, monster, target);
	}
}

static void try_attack(player_struct *monster, struct SkillTable *config, struct ai_player_data *ai_player_data, player_struct **enemy)
{
	assert(config && config->SkillType == 2);

	std::vector<unit_struct *> target;

	if (count_skill_hit_unit(monster->get_pos(), ai_player_data->angle, ai_player_data->skill_id, &target, enemy) != 0)
	{
		LOG_DEBUG("aitest: [%s]can not find target", monster->get_name());
		return;
	}
	ai_player_hit_notify_to_many_player(config->ID, monster, &target);
}

static player_struct **get_enemy_team_player(player_struct *player, struct ai_player_data **ai_player_data)
{
	raid_struct *raid = (raid_struct *)player->scene;
	assert(raid->m_config->DengeonRank == DUNGEON_TYPE_PVP_3 || raid->m_config->DengeonRank == DUNGEON_TYPE_PVP_5);

	player_struct **enemy = NULL;
	*ai_player_data = NULL;
	for (int i = 0; i < MAX_TEAM_MEM; ++i)
	{
		if (player == raid->m_player[i])
		{
			enemy = raid->m_player2;
			*ai_player_data = &raid->PVP_DATA.ai_player_data[i];
			return enemy;
		}
		if (player == raid->m_player2[i])
		{
			enemy = raid->m_player;
			*ai_player_data = &raid->PVP_DATA.ai_player_data[i + MAX_TEAM_MEM];
			return enemy;
		}
	}
	return NULL;
}

static void do_ai_player_relive(raid_struct *raid, player_struct *player, struct ai_player_data *ai_player_data, player_struct **enemy)
{
	if (ai_player_data->relive_time == 0)
	{
		ai_player_data->relive_time = time_helper::get_cached_time() / 1000 + sg_pvp_raid_relive_cd;
		return;
	}
	if (time_helper::get_cached_time() / 1000 < ai_player_data->relive_time)
		return;

	ai_player_data->relive_time = 0;
	player->data->attrData[PLAYER_ATTR_HP] = player->data->attrData[PLAYER_ATTR_MAXHP];

	player->on_hp_changed(0);

	int32_t pos_x, pos_z, direct;
	pvp_raid_get_relive_pos(raid, &pos_x, &pos_z, &direct);
	reset_patrol_index(player);

	LOG_INFO("%s: ai player[%lu] relive to pos[%d][%d]", __FUNCTION__, player->get_uuid(), pos_x, pos_z);
	
	scene_struct *t_scene = raid;
	raid->delete_player_from_scene(player);
	player->set_pos(pos_x, pos_z);
	int camp_id = player->get_camp_id();
	t_scene->add_player_to_scene(player);
	player->set_camp_id(camp_id);

	if (player->m_team)
		player->m_team->OnMemberHpChange(*player);

		//复活的时候加上一个无敌buff
	buff_manager::create_default_buff(114400001, player, player, false);	
}

static void do_ai_player_attack(player_struct *monster, struct ai_player_data *ai_player_data, player_struct **enemy)
{
	ai_player_data->ai_state = AI_PURSUE_STATE;

	if (!monster->data)
		return;

	// if (!monster->target || !monster->target->is_avaliable())
	// {
	//	monster->ai_state = AI_PATROL_STATE;
	//	return;
	// }

	struct SkillTable *config = get_config_by_id(ai_player_data->skill_id, &skill_config);

	LOG_DEBUG("aitest: [%s]skill = %d", monster->get_name(), ai_player_data->skill_id);

	monster->reset_pos();

	if (!config)
		return;
	struct ActiveSkillTable *act_config = get_config_by_id(config->SkillAffectId, &active_skill_config);
	if (act_config)
	{

		ai_player_data->ontick_time += act_config->TotalSkillDelay;
		LOG_DEBUG("aitest: %s %d: [%s]onticktime += %lu", __FUNCTION__, __LINE__, monster->get_name(), act_config->TotalSkillDelay);
		// for (size_t i = 0; i < act_config->n_SkillLength; ++i)
		// {
		// 	ai_player_data->ontick_time += act_config->SkillLength[i];
		// 	LOG_DEBUG("aitest: %s %d: [%s]onticktime += %u", __FUNCTION__, __LINE__, monster->get_name(), act_config->SkillLength[i]);					
		// }
	}

	// if (config->MaxCount <= 1)
	// {
	//	try_attack_target(monster, , config);
	// }
	// else
	{
		try_attack(monster, config, ai_player_data, enemy);
	}

	ai_player_data->skill_id = 0;
}

void install_ai_player_handle()
{
	player_manager::m_ai_player_handle = pvp_player_ai_tick;
}
void uninstall_ai_player_handle()
{
	player_manager::m_ai_player_handle = NULL;
}
