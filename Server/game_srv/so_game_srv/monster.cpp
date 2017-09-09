#include <assert.h>
#include <math.h>
#include "game_event.h"
#include "check_range.h"
#include "game_config.h"
#include "monster.h"
#include "scene_manager.h"
#include "raid_manager.h"
#include "time_helper.h"
#include "camp_judge.h"
#include "player_manager.h"
#include "partner_manager.h"
#include "cash_truck_manager.h"
#include "monster_manager.h"
#include "uuid.h"
#include "msgid.h"
#include "raid.h"
#include "scene.h"
#include "attr_calc.h"
#include "sortarray.h"
#include "unit.h"
#include "cash_truck.h"
#include "buff.h"
#include "app_data_statis.h"
#include "cached_hit_effect.h"
#include "count_skill_damage.h"
#include "zhenying_raid_manager.h"
#include "zhenying_battle.h"
#include "sight_space_manager.h"
#include "monster_ai.h"
#include "collect.h"
#include "game_config.h"
#include "../proto/player_redis_info.pb-c.h"
#include "chat.pb-c.h"

uint64_t monster_struct::get_uuid()
{
	return data->player_id;
}
uint32_t monster_struct::get_skill_id()
{
	return data->skill_id;
}

double *monster_struct::get_all_attr()
{
	return &data->attrData[0];
}
double monster_struct::get_attr(uint32_t id)
{
	return data->attrData[id];
}
double *monster_struct::get_all_buff_fight_attr()
{
	return &data->buff_fight_attr[0];
}
double monster_struct::get_buff_fight_attr(uint32_t id)
{
	assert(id < MAX_BUFF_FIGHT_ATTR);
	return data->buff_fight_attr[id];
}

void monster_struct::set_attr(uint32_t id, double value)
{
	assert(id < PLAYER_ATTR_MAX);
	data->attrData[id] = value;
}

struct unit_path *monster_struct::get_unit_path()
{
	return &data->move_path;
}
float monster_struct::get_speed()
{
	return data->attrData[PLAYER_ATTR_MOVE_SPEED];
}

UNIT_TYPE monster_struct::get_unit_type()
{
	if (config->HateType == MONSTER_HATETYPE_DEFINE_BOSS
		|| config->HateType == MONSTER_HATETYPE_DEFINE_AIBOSS)
		return UNIT_TYPE_BOSS;
	return UNIT_TYPE_MONSTER;
}

bool monster_struct::is_avaliable()
{
	return data != NULL && scene != NULL;
}

int *monster_struct::get_cur_sight_player()
{
	return &data->cur_sight_player;
}
uint64_t *monster_struct::get_all_sight_player()
{
	return &data->sight_player[0];	
}
int *monster_struct::get_cur_sight_monster()
{
	return &data->cur_sight_monster;
}
uint64_t *monster_struct::get_all_sight_monster()
{
	return &data->sight_monster[0];	
}
int *monster_struct::get_cur_sight_truck()
{
	return &data->cur_sight_truck;
}
uint64_t *monster_struct::get_all_sight_truck()
{
	return &data->sight_truck[0];
}
int *monster_struct::get_cur_sight_partner()
{
	return &data->cur_sight_partner;
}
uint64_t *monster_struct::get_all_sight_partner()
{
	return &data->sight_partner[0];
}

JobDefine monster_struct::get_job()
{
	return JOB_DEFINE_MONSTER;
}


bool monster_struct::can_see_player(player_struct *player)
{
	return player->is_monster_in_sight(data->player_id);
}

void monster_struct::on_tick()
{
	update_monster_pos_and_sight();

	pos_changed = false;
	if (pos_changed && !watched_player_id.empty())
	{
		SpecialMonsterPosNotify nty;
		EXTERN_DATA extern_data;	
		special_monster_pos_notify__init(&nty);

		for (std::list<uint64_t>::iterator ite = watched_player_id.begin(); ite != watched_player_id.end(); ++ite)
		{
			player_struct *player = player_manager::get_player_by_id(*ite);
			if (!player)
				continue;
			struct position *pos = get_pos();
			nty.monster_id = data->monster_id;
			nty.scene_id = data->scene_id;
			nty.uuid = data->player_id;
			nty.pos_x = pos->pos_x;
			nty.pos_z = pos->pos_z;
			extern_data.player_id = player->get_uuid();
			fast_send_msg(&conn_node_gamesrv::connecter, &extern_data,
				MSG_ID_SPECIAL_MONSTER_POS_NOTIFY, special_monster_pos_notify__pack, nty);
		}
	}

	if (buff_state & BUFF_STATE_ONEBLOOD)
	{
		return;
	}
	
	if (buff_state & BUFF_STATE_TAUNT)
	{
		return do_taunt_action();
	}

	if (ai)
		ai->on_tick(this);
}

void monster_struct::on_region_changed(uint16_t old_region_id, uint16_t new_region_id)
{
	raid_struct *raid = get_raid();
	if (raid && raid->ai && raid->ai->raid_on_monster_region_changed)
	{
		raid->ai->raid_on_monster_region_changed(raid, this, old_region_id, new_region_id);
	}
}

void monster_struct::update_region_id()
{
	if (!is_avaliable())
		return;
	struct position *pos = get_pos();
	uint16_t old_region_id = get_attr(PLAYER_ATTR_REGION_ID);
	uint16_t new_region_id = get_region_id(scene->map_config, scene->region_config, pos->pos_x, pos->pos_z);
	if (new_region_id != old_region_id)
	{
		set_attr(PLAYER_ATTR_REGION_ID, new_region_id);
//		send_enter_region_notify(new_region_id);
		on_region_changed(old_region_id, new_region_id);
	}
}

void monster_struct::update_monster_pos_and_sight()
{
	if (!scene)
		return;
	if (data->attrData[PLAYER_ATTR_MOVE_SPEED] == 0)
		return;
	if (!is_unit_in_move())
		return;

//	float speed = data->attrData[PLAYER_ATTR_MOVE_SPEED];
//	if (ai_state == AI_GO_BACK_STATE)
//		speed *= 2;
	
	area_struct *old_area = area;
	if (update_unit_position() == 0)
		return;

	update_region_id();	

	if (!old_area)
		return;
	
	struct position *pos = get_pos();	
	area_struct *new_area = scene->get_area_by_pos(pos->pos_x, pos->pos_z);
		//检查是否越过area
	if (old_area == new_area || !new_area)
		return;

	update_sight(old_area, new_area);
}

void monster_struct::reset_timer(uint64_t time)
{
	data->ontick_time = time;
	monster_manager::monster_ontick_reset_timer(this);
}

void monster_struct::set_timer(uint64_t time)
{
	data->ontick_time = time;
	monster_manager::monster_ontick_settimer(this);
}

void monster_struct::on_go_back()
{	
	target = NULL;
	if (config->HateType == MONSTER_HATETYPE_DEFINE_BOSS)
	{
		memset(&hate_unit[0], 0, sizeof(hate_unit));	
	}
}
void monster_struct::on_pursue()
{
	if (config->HateType == MONSTER_HATETYPE_DEFINE_BOSS)
	{	
		uint64_t now = time_helper::get_cached_time();
		if (now < next_hate_reduce_time)
		{
			return;
		}

		int n = (now - next_hate_reduce_time) / 1000 + 1;
		next_hate_reduce_time += n * 1000;
		double delta = 0.9 * n;
		for (int i = 0; i < MAX_HATE_UNIT; ++i)
		{
			if (hate_unit[i].uuid == 0)
				continue;
			hate_unit[i].hate_value *= delta;
		}
	}
}

void monster_struct::clear_monster_timer()
{
	monster_manager::monster_ontick_delete(this);
}

void monster_struct::calculate_attribute(void)
{
	std::map<uint64_t, struct MonsterTable *>::iterator ite = monster_config.find(data->monster_id);
	if (ite == monster_config.end())
		return;
	config = ite->second;
	::get_attr_from_config(config->BaseAttribute * 1000 + get_attr(PLAYER_ATTR_LEVEL), data->attrData, &drop_id);
	data->attrData[PLAYER_ATTR_HP] = data->attrData[PLAYER_ATTR_MAXHP];
//	data->speed = data->attrData[PLAYER_ATTR_MOVE_SPEED] + ai_config->MovingChange / 100.0;
	data->attrData[PLAYER_ATTR_MOVE_SPEED] += (int)(ai_config->MovingChange) / 100.0;
	assert(data->ontick_time == 0);
	set_timer(time_helper::get_cached_time() + ai_config->Response + random() % 200);
	calculate_buff_fight_attr(true);	
}

monster_struct::~monster_struct()
{
}

void monster_struct::init_monster()
{
	init_unit_struct();
	scene = NULL;
	area = NULL;
	config = NULL;
	create_config = NULL;
	ai_config = NULL;
	lock_time = 0;
	drop_id = 0;
	buff_state = 0;
	target = NULL;

	sight_space = NULL;
	mark_delete = false;
	
	ai = NULL;
//	ai_type = AI_TYPE_NORMAL;
//	ai_state = AI_PATROL_STATE;
//	set_ai_interface(1);
	memset(&ai_data, 0, sizeof(ai_data));
	memset(&m_buffs[0], 0, sizeof(m_buffs));
	
	memset(&hate_unit[0], 0, sizeof(hate_unit));
	next_hate_reduce_time = time_helper::get_cached_time() + 1000;
}

extern struct ai_interface *all_monster_ai_interface[MAX_MONSTER_AI_INTERFACE];
void monster_struct::set_ai_interface(int ai_type)
{
	if (ai_type >= 0 && ai_type < MAX_MONSTER_AI_INTERFACE)
		ai = all_monster_ai_interface[ai_type];
	else
		ai = NULL;
}

void monster_struct::add_ai_interface(int ai_type, struct ai_interface *ai)
{
	assert(ai_type >= 0 && ai_type < MAX_MONSTER_AI_INTERFACE);
	if (ai)
	{	
//	assert(ai->on_beattack);
		assert(ai->on_dead);
//	assert(ai->on_fly);
		assert(ai->on_tick);
	}
	all_monster_ai_interface[ai_type] = ai;
}

void monster_struct::pack_sight_monster_info(SightMonsterInfo *info)
{
	sight_monster_info__init(info);
	info->hp = data->attrData[PLAYER_ATTR_HP];
	info->uuid = data->player_id;
	info->monsterid = data->monster_id;
	info->lv = data->attrData[PLAYER_ATTR_LEVEL];
	info->pk_type = data->attrData[PLAYER_ATTR_PK_TYPE];
	info->zhenying_id = data->attrData[PLAYER_ATTR_ZHENYING];	

//	info->direct_x = data->move_path.speed_x;
//	info->direct_y = 0;
//	info->direct_z = data->move_path.speed_z;
	info->data = pack_unit_move_path(&info->n_data);
	info->speed = data->attrData[PLAYER_ATTR_MOVE_SPEED];
	info->buff_info = pack_unit_buff(&info->n_buff_info);
	info->direct = data->born_direct;
/*	
	info->n_data = 1;
	info->data = &pos_pool_pos_point[pos_pool_len];

	pos_data__init(pos_pool_pos_point[pos_pool_len]);
	pos_pool_pos[pos_pool_len].pos_x = data->move_path.pos[data->move_path.cur_pos].pos_x;
	pos_pool_pos[pos_pool_len].pos_z = data->move_path.pos[data->move_path.cur_pos].pos_z;
*/	
}

void monster_struct::add_sight_space_player_to_sight(sight_space_struct *sight_space, uint16_t *add_player_id_index, uint64_t *add_player)
{
	for (int j = 0; j < MAX_PLAYER_IN_SIGHT_SPACE; ++j)
	{
		player_struct *player = sight_space->players[j];
		if (!player)
			continue;
		if (player->add_monster_to_sight_both(this) >= 0)
		{
			add_player[*add_player_id_index] = player->data->player_id;
			(*add_player_id_index)++;
		}
	}
}

void monster_struct::add_sight_space_truck_to_sight(sight_space_struct *sight_space)
{
	for (int j = 0; j < MAX_PLAYER_IN_SIGHT_SPACE; ++j)
	{
		cash_truck_struct *truck = sight_space->trucks[j];
		if (!truck)
			continue;

		add_truck_to_sight_both(truck);
	}
}

void monster_struct::add_sight_space_partner_to_sight(sight_space_struct *sight_space)
{
	for (int j = 0; j < MAX_PARTNER_IN_SIGHT_SPACE; ++j)
	{
		partner_struct *partner = sight_space->partners[j];
		if (!partner)
			continue;

		add_partner_to_sight_both(partner);
	}
}
void monster_struct::add_sight_space_monster_to_sight(sight_space_struct *sight_space)
{
	for (int j = 0; j < MAX_MONSTER_IN_SIGHT_SPACE; ++j)
	{
		monster_struct *monster = sight_space->monsters[j];
		if (!monster)
			continue;

		if (monster == this)
			continue;
		add_monster_to_sight_both(monster);
	}
}

void monster_struct::add_area_player_to_sight(area_struct *area, uint16_t *add_player_id_index, uint64_t *add_player)
{
	if (!area)
		return;
	for (int j = 0; j < area->cur_player_num; ++j)
	{
		player_struct *player = player_manager::get_player_by_id(area->m_player_ids[j]);
		if (!player)
		{
				//在视野里面居然找不到？  bug ？
			LOG_ERR("%s %d: can not find sight player %lu area[%p]", __FUNCTION__, __LINE__, area->m_player_ids[j], area);				
			continue;
		}
		if (player->add_monster_to_sight_both(this) >= 0)
		{
			if (get_entity_type(area->m_player_ids[j]) == ENTITY_TYPE_PLAYER)
			{
				add_player[*add_player_id_index] = player->data->player_id;
				(*add_player_id_index)++;
			}
		}
	}
}

void monster_struct::add_area_truck_to_sight(area_struct *area)
{
	if (!area)
		return;
	for (int j = 0; j < area->cur_truck_num; ++j)
	{
		if (area->m_truck_uuid[j] == data->player_id)
			continue;
		
		cash_truck_struct *truck = cash_truck_manager::get_cash_truck_by_id(area->m_truck_uuid[j]);
		if (!truck)
		{
				//在视野里面居然找不到？  bug ？
			LOG_ERR("%s %d: can not find sight truck %lu area[%p]", __FUNCTION__, __LINE__, area->m_truck_uuid[j], area);				
			continue;
		}
		add_truck_to_sight_both(truck);
	}
}

void monster_struct::add_area_monster_to_sight(area_struct *area)
{
	if (!area)
		return;
	for (int j = 0; j < area->cur_monster_num; ++j)
	{
		if (area->m_monster_uuid[j] == data->player_id)
			continue;
		
		monster_struct *monster = monster_manager::get_monster_by_id(area->m_monster_uuid[j]);
		if (!monster)
		{
				//在视野里面居然找不到？  bug ？
			LOG_ERR("%s %d: can not find sight player %lu area[%p]", __FUNCTION__, __LINE__, area->m_monster_uuid[j], area);				
			continue;
		}
		add_monster_to_sight_both(monster);
	}
}
void monster_struct::add_area_partner_to_sight(area_struct *area)
{
	if (!area)
		return;
	for (int j = 0; j < area->cur_partner_num; ++j)
	{
		if (area->m_partner_uuid[j] == data->player_id)
			continue;
		
		partner_struct *partner = partner_manager::get_partner_by_uuid(area->m_partner_uuid[j]);
		if (!partner)
		{
				//在视野里面居然找不到？  bug ？
			LOG_ERR("%s %d: can not find sight partner %lu area[%p]", __FUNCTION__, __LINE__, area->m_partner_uuid[j], area);				
			continue;
		}
		add_partner_to_sight_both(partner);
	}
}

int monster_struct::broadcast_monster_move()
{
	if (!area && !sight_space)
		return (0);
	
	MoveNotify notify;
	move_notify__init(&notify);
	notify.playerid = data->player_id;
	notify.data = pack_unit_move_path(&notify.n_data);

	broadcast_to_sight(MSG_ID_MOVE_NOTIFY, &notify, (pack_func)move_notify__pack, false);
	reset_pos_pool();
	return (0);
}

bool monster_struct::is_in_safe_region()
{
	return false;
}

bool monster_struct::can_beattack()
{
	if (buff_state & BUFF_STATE_GOD)
		return false;
	if (config->AttackType == 2)
		return false;
	return true;
}

// int monster_struct::get_pk_type()
// {
// //怪物模式	
// 	return 3;
// }

int monster_struct::add_skill_cd(uint32_t index, uint64_t now)
{
	if (index >= MAX_MONSTER_SKILL)
		return (0);

	if (index >= config->n_Skill)
		return (0);

	uint32_t skill_id = config->Skill[index];
	SkillTable *config = get_config_by_id(skill_id, &skill_config);
	if (!config)
		return (0);
	
	struct SkillLvTable *lv_config = get_config_by_id(config->SkillLv, &skill_lv_config);
	if (!lv_config)
		return (0);
	skill_cd[index] = now + lv_config->CD;
	LOG_INFO("%s: %p add index[%u] cd[%lu]", __FUNCTION__, this, index, skill_cd[index]);
	return (0); 
}

bool monster_struct::is_skill_in_cd(uint32_t index, uint64_t now)
{
	if (index >= MAX_MONSTER_SKILL)	
		return false;
	return now < skill_cd[index];
}

Team *monster_struct::get_team()
{
	return NULL;
}

int monster_struct::get_camp_id()
{
	return data->camp_id;
}

void monster_struct::set_camp_id(int id)
{
	data->camp_id = id;
}

int monster_struct::broadcast_monster_create(uint32_t effectid)
{
	if (!area)
		return (0);

	LOG_DEBUG("%s %d: create monster %u %lu at area %ld[%.1f][%.1f]", __FUNCTION__, __LINE__, data->monster_id, get_uuid(),
		area - scene->m_area, get_pos()->pos_x, get_pos()->pos_z);
	
	SightChangedNotify notify;
	sight_changed_notify__init(&notify);
	SightMonsterInfo monster_info[1];
	SightMonsterInfo *monster_info_point[1];
	monster_info_point[0] = &monster_info[0];
	notify.n_add_monster = 1;
	notify.add_monster = monster_info_point;
	pack_sight_monster_info(monster_info_point[0]);
	monster_info_point[0]->effectid = effectid;

	uint64_t *ppp = conn_node_gamesrv::prepare_broadcast_msg_to_players(MSG_ID_SIGHT_CHANGED_NOTIFY, &notify, (pack_func)sight_changed_notify__pack);
	PROTO_HEAD_CONN_BROADCAST *head;	
	head = (PROTO_HEAD_CONN_BROADCAST *)conn_node_base::global_send_buf;	

	add_area_player_to_sight(area, &head->num_player_id, ppp);
	for (int i = 0; i < MAX_NEIGHBOUR_AREA; ++i)
	{
		add_area_player_to_sight(area->neighbour[i], &head->num_player_id, ppp);
	}

	add_area_monster_to_sight(area);
	for (int i = 0; i < MAX_NEIGHBOUR_AREA; ++i)
	{
		add_area_monster_to_sight(area->neighbour[i]);
	}

	add_area_truck_to_sight(area);
	for (int i = 0; i < MAX_NEIGHBOUR_AREA; ++i)
	{
		add_area_truck_to_sight(area->neighbour[i]);
	}
	add_area_partner_to_sight(area);
	for (int i = 0; i < MAX_NEIGHBOUR_AREA; ++i)
	{
		add_area_partner_to_sight(area->neighbour[i]);
	}
	
	if (head->num_player_id > 0)
	{
		head->len += sizeof(uint64_t) * head->num_player_id;	
		conn_node_gamesrv::broadcast_msg_send();
	}
	reset_pools();
	
	return (0);
}
/*
int monster_struct::add_player_to_sight(uint64_t player_id)
{
	LOG_DEBUG("%s : %p %lu %u", __FUNCTION__, this, player_id, data->cur_sight_player);
	return array_insert(&player_id, &data->sight_player[0], &data->cur_sight_player, sizeof(uint64_t), 1, comp_uint64);	
}
int monster_struct::del_player_from_sight(uint64_t player_id)
{
	LOG_DEBUG("%s : %p %lu %u", __FUNCTION__, this, player_id, data->cur_sight_player);
	return array_delete(&player_id, &data->sight_player[0], &data->cur_sight_player, sizeof(uint64_t), comp_uint64);	
}
*/

void monster_struct::on_beattack(unit_struct *player, uint32_t skill_id, int32_t damage)
{
	if (!data || !scene)
		return;

	if (damage > 0)
	{
		count_hate(player, skill_id, damage);
		update_target();
	}	
	
	if (ai && ai->on_beattack)
		ai->on_beattack(this, player);

}

void monster_struct::on_repel(unit_struct *player)
{
	if (!data || !scene)
		return;
	
	if (ai && ai->on_fly)
		ai->on_fly(this, player);
}

void monster_struct::on_relive()
{
	LOG_DEBUG("%s: %lu[%p] relive", __FUNCTION__, get_uuid(), this);
	if (!scene)
	{
		if (data->raid_uuid)
		{
			DungeonTable* config = get_config_by_id(data->scene_id, &all_raid_config);
			if (config != NULL && config->DengeonRank == DUNGEON_TYPE_ZHENYING)
			{
				scene = zhenying_raid_manager::get_zhenying_raid_by_uuid(data->raid_uuid);
			}
			else
			{
				scene = raid_manager::get_raid_by_uuid(data->raid_uuid);
			}
		}

		else
		{
			scene = scene_manager::get_scene(data->scene_id);
		}
	}

	assert(scene);
	scene->add_monster_to_scene(this, 0);
	//	broadcast_monster_create();
}

void monster_struct::on_hp_changed(int damage)
{
	if (damage <= 0)
		return;
	if (config->talk_config)
	{
		double max_hp = get_attr(PLAYER_ATTR_MAXHP);
		double cur_hp = get_attr(PLAYER_ATTR_HP);
		uint32_t old_percent = (int)((cur_hp + damage) / max_hp * 100);
		uint32_t percent = get_monster_hp_percent(this);
		struct NpcTalkTable *talk_config;
		for (talk_config = config->talk_config; talk_config; talk_config = talk_config->next)
		{
			if (talk_config->Type != 2)
				continue;
			if (talk_config->EventNum1 != 2)
				continue;
			assert(talk_config->n_EventNum2 > 0);
			if (percent > talk_config->EventNum2[0])
				continue;
			if (old_percent <= talk_config->EventNum2[0])
				continue;
			
			MonsterTalkNotify nty;
			monster_talk_notify__init(&nty);
			nty.talkid = talk_config->ID;
			nty.uuid = get_uuid();
			broadcast_to_sight(MSG_ID_MONSTER_TALK_NOTIFY, &nty, (pack_func)monster_talk_notify__pack, false);
			break;
		}
	}
	
	if (ai && ai->on_hp_changed)
	{
		ai->on_hp_changed(this);
	}
}

void monster_struct::on_dead(unit_struct *killer)
{
	if (!data || !scene)
		return;

	bool player_kill = (killer && killer->get_unit_type() == UNIT_TYPE_PLAYER);

	LOG_DEBUG("%s: %lu[%u][%p] dead", __FUNCTION__, get_uuid(), data->monster_id, this);

	clear_all_buffs();

	uint32_t scene_id = scene->m_id;
	uint32_t monster_id = data->monster_id;
	//特定怪物死亡创建采集点
	monster_dead_creat_collect(killer);

	if (killer && drop_id > 0) //todo 国御目标怪根据任务掉落
		killer->give_drop_item(drop_id, MAGIC_TYPE_MONSTER_DEAD, ADD_ITEM_AS_MUCH_AS_POSSIBLE);
		
	data->relive_time = ai_config->Regeneration * 1000 + time_helper::get_cached_time();
	scene_struct *o_scene = scene;
	scene->delete_monster_from_scene(this, false);
//	broadcast_monster_delete();
	if (ai)
		ai->on_dead(this, o_scene);

	o_scene->on_monster_dead(this, killer);

	if (sight_space != NULL)
	{
		sight_space->broadcast_monster_delete(this);
		if (sight_space->data->n_monster_uuid == 0)
		{
			if (player_kill)
			{
				if (sight_space->data->type != 0)
				{
					sight_space_manager::del_player_from_sight_space(sight_space, (player_struct *)killer, true);
					return;
				}
			}
		}
	}

		//任务可能会导致怪物被删除，放在最后
	if (player_kill)
	{
		((player_struct*)killer)->add_partner_anger(1, true);
		((player_struct*)killer)->add_task_progress(TCT_KILL_MONSTER, monster_id, 1);
		((player_struct*)killer)->touch_task_drop(scene_id, monster_id);
	}
	else if (sight_space != NULL && sight_space->data->type == 0)
	{
		std::vector<player_struct*> players;
		//任务位面副本，友方怪杀死的怪物，也算进任务计数
		for (int i = 0; i < MAX_PLAYER_IN_SIGHT_SPACE; ++i)
		{
			player_struct *player = sight_space->players[i];
			if (player == NULL)
			{
				continue;
			}

			if (get_unit_fight_type(this, player) != UNIT_FIGHT_TYPE_ENEMY)							
			{
				continue;
			}

			players.push_back(player);
		}

		for (std::vector<player_struct*>::iterator iter = players.begin(); iter != players.end(); ++iter)
		{
			(*iter)->add_task_progress(TCT_KILL_MONSTER, monster_id, 1);
			(*iter)->touch_task_drop(scene_id, monster_id);
		}
	}
}

int monster_struct::del_truck_from_sight_both(cash_truck_struct *truck)
{
	int ret = del_truck_from_sight(truck->data->player_id);
	if (ret >= 0)
	{
		int ret1 = truck->del_monster_from_sight(data->player_id);
		assert(ret1 >= 0);
	}
	return ret;
}
int monster_struct::add_truck_to_sight_both(cash_truck_struct *truck)
{
			//友好，中立关系的不用加
	if (get_unit_fight_type(this, truck) != UNIT_FIGHT_TYPE_ENEMY
		&& get_unit_fight_type(truck, this) != UNIT_FIGHT_TYPE_ENEMY)
		return (-1);
	
	if (prepare_add_truck_to_sight(truck) != 0 ||
		truck->prepare_add_monster_to_sight(this) != 0)
		return -1;
	
	int ret = add_truck_to_sight(truck->data->player_id);
	assert (ret >= 0);
	int ret1 = truck->add_monster_to_sight(data->player_id);
	assert(ret1 >= 0);		
	return ret;
}

int monster_struct::add_monster_to_sight_both(monster_struct *monster)
{
			//友好，中立关系的不用加
	if (get_unit_fight_type(this, monster) != UNIT_FIGHT_TYPE_ENEMY
		&& get_unit_fight_type(monster, this) != UNIT_FIGHT_TYPE_ENEMY)
		return (-1);
	
	if (prepare_add_monster_to_sight(monster) != 0 ||
		monster->prepare_add_monster_to_sight(this) != 0)
		return -1;
	
	int ret = add_monster_to_sight(monster->data->player_id);
	assert (ret >= 0);
	int ret1 = monster->add_monster_to_sight(data->player_id);
	assert(ret1 >= 0);		
	return ret;
}

int monster_struct::del_monster_from_sight_both(monster_struct *monster)
{
	int ret = del_monster_from_sight(monster->data->player_id);
	if (ret >= 0)
	{
		int ret1 = monster->del_monster_from_sight(data->player_id);
		assert(ret1 >= 0);
	}
	return ret;
}

int monster_struct::add_partner_to_sight_both(partner_struct *partner)
{
			//友好，中立关系的不用加
	if (get_unit_fight_type(this, partner) != UNIT_FIGHT_TYPE_ENEMY
		&& get_unit_fight_type(partner, this) != UNIT_FIGHT_TYPE_ENEMY)
		return (-1);
	
	if (prepare_add_partner_to_sight(partner) != 0 ||
		partner->prepare_add_monster_to_sight(this) != 0)
		return -1;
	
	int ret = add_partner_to_sight(partner->data->uuid);
	assert (ret >= 0);
	int ret1 = partner->add_monster_to_sight(data->player_id);
	assert(ret1 >= 0);		
	return ret;
}

int monster_struct::del_partner_from_sight_both(partner_struct *partner)
{
	int ret = del_partner_from_sight(partner->data->uuid);
	if (ret >= 0)
	{
		int ret1 = partner->del_monster_from_sight(data->player_id);
		assert(ret1 >= 0);
	}
	return ret;
}


int monster_struct::prepare_add_player_to_sight(player_struct *player)
{
	if (data->cur_sight_player < MAX_PLAYER_IN_MONSTER_SIGHT)
		return (0);

//todo 检查关系的优先级，然后删除低优先级的来腾出空间
	return (-1);
}
int monster_struct::prepare_add_truck_to_sight(cash_truck_struct * truck)
{
	// 	//友好，中立关系的不用加
	// if (get_unit_fight_type(this, truck) != UNIT_FIGHT_TYPE_ENEMY)
	// 	return (-1);
	
	if (data->cur_sight_truck < MAX_TRUCK_IN_MONSTER_SIGHT)
		return (0);

//todo 检查关系的优先级，然后删除低优先级的来腾出空间
	return (-1);
}

int monster_struct::prepare_add_partner_to_sight(partner_struct *partner)
{
	// 	//友好，中立关系的不用加
	// if (get_unit_fight_type(this, partner) != UNIT_FIGHT_TYPE_ENEMY)
	// 	return (-1);
	
		//死了的不进入视野
//	if (partner->data->attrData[PLAYER_ATTR_HP] <= 0)
//		return (-1);
	
	if (data->cur_sight_partner < MAX_PARTNER_IN_MONSTER_SIGHT)
		return (0);

//todo 检查关系的优先级，然后删除低优先级的来腾出空间
	return (-1);
}

int monster_struct::prepare_add_monster_to_sight(monster_struct *monster)
{
	// 	//友好，中立关系的不用加
	// if (get_unit_fight_type(this, monster) != UNIT_FIGHT_TYPE_ENEMY)
	// 	return (-1);
	
		//死了的不进入视野
//	if (monster->data->attrData[PLAYER_ATTR_HP] <= 0)
//		return (-1);
	
	if (data->cur_sight_monster < MAX_MONSTER_IN_MONSTER_SIGHT)
		return (0);

//todo 检查关系的优先级，然后删除低优先级的来腾出空间
	return (-1);
}


int monster_struct::broadcast_monster_delete(bool send_msg)
{
	assert(data);

	for (int i = 0; i < data->cur_sight_player; ++i)
	{
		player_struct *player = player_manager::get_player_by_id(data->sight_player[i]);
		if (player)
			player->del_monster_from_sight(data->player_id);
	}
	for (int i = 0; i < data->cur_sight_monster; ++i)
	{
		monster_struct *monster = monster_manager::get_monster_by_id(data->sight_monster[i]);
		if (monster)
		{
			LOG_DEBUG("%s: %lu[%u] del monster %lu[%u]", __FUNCTION__, get_uuid(), data->cur_sight_monster,
				monster->get_uuid(), monster->data->cur_sight_monster);		
			monster->del_monster_from_sight(data->player_id);
		}
		else
		{
			LOG_ERR("%s: %lu can not find sight monster %lu", __FUNCTION__, get_uuid(), data->sight_monster[i]);
		}
	}
	for (int i = 0; i < data->cur_sight_truck; ++i)
	{
		cash_truck_struct *truck = cash_truck_manager::get_cash_truck_by_id(data->sight_truck[i]);
		if (truck)
		{
			truck->del_monster_from_sight(data->player_id);
		}
		else
		{
			LOG_ERR("%s: %lu can not find sight truck %lu", __FUNCTION__, get_uuid(), data->sight_truck[i]);
		}
	}
	for (int i = 0; i < data->cur_sight_partner; ++i)
	{
		partner_struct *partner = partner_manager::get_partner_by_uuid(data->sight_partner[i]);
		if (partner)
		{
			partner->del_monster_from_sight(data->player_id);
		}
		else
		{
			LOG_ERR("%s: %lu can not find sight partner %lu", __FUNCTION__, get_uuid(), data->sight_partner[i]);
		}
	}

	SightChangedNotify notify;
	uint64_t del_uuid[1];
	uint64_t *ppp;
	
	if (!send_msg)
		goto done;
	
	sight_changed_notify__init(&notify);
	del_uuid[0] = data->player_id;
	notify.delete_monster = del_uuid;
		//发送给需要在视野里面添加玩家的通知
	notify.n_delete_monster = 1;
	ppp = conn_node_gamesrv::prepare_broadcast_msg_to_players(MSG_ID_SIGHT_CHANGED_NOTIFY, &notify, (pack_func)sight_changed_notify__pack);
	for (int i = 0; i < data->cur_sight_player; ++i)
		conn_node_gamesrv::broadcast_msg_add_players(data->sight_player[i], ppp);
	conn_node_gamesrv::broadcast_msg_send();

done:
	clear_monster_sight();
	return (0);
}

void monster_struct::clear_monster_sight()
{
	LOG_DEBUG("%s: %lu", __FUNCTION__, get_uuid());
	data->cur_sight_player = 0;
	data->cur_sight_monster = 0;
	data->cur_sight_truck = 0;
	data->cur_sight_partner = 0;		
}

void monster_struct::del_sight_player_in_area(int n_del, area_struct **del_area, int *delete_player_id_index, uint64_t *delete_player_id)
{
	int i, j;
	int index = 0;
	player_struct *del_sight_player[MAX_PLAYER_IN_PLAYER_SIGHT];	
	
	for (i = 0; i < data->cur_sight_player; ++i)
	{
		player_struct *player = player_manager::get_player_by_id(data->sight_player[i]);
		if (!player)
		{
				//在视野里面居然找不到？  bug ？
			LOG_ERR("%s %d: %lu can not find sight player %lu", __FUNCTION__, __LINE__, data->player_id, data->sight_player[i]);
			continue;
		}
		for (j = 0; j < n_del; ++j)
		{
			if (player->area != del_area[j])
				continue;
			delete_player_id[(*delete_player_id_index)++] = player->data->player_id;			
			del_sight_player[index++] = player;						
		}
	}

	for (i = 0; i < index; ++i)	
		del_sight_player[i]->del_monster_from_sight_both(this);
}

void monster_struct::del_sight_truck_in_area(int n_del, area_struct **del_area)
{
	int i, j;
	int index = 0;
	cash_truck_struct *del_sight_truck[MAX_TRUCK_IN_MONSTER_SIGHT];
	
	for (i = 0; i < data->cur_sight_truck; ++i)
	{
		cash_truck_struct *truck = cash_truck_manager::get_cash_truck_by_id(data->sight_truck[i]);
		if (!truck)
		{
				//在视野里面居然找不到？  bug ？
			LOG_ERR("%s %d: %lu can not find sight truck %lu", __FUNCTION__, __LINE__, data->player_id, data->sight_truck[i]);
			continue;
		}
		for (j = 0; j < n_del; ++j)
		{
			if (truck->area != del_area[j])
				continue;
			del_sight_truck[index++] = truck;
		}
	}

	for (i = 0; i < index; ++i)
		del_truck_from_sight_both(del_sight_truck[i]);
}

void monster_struct::del_sight_monster_in_area(int n_del, area_struct **del_area)
{
	int i, j;
	int index = 0;
	monster_struct *del_sight_monster[MAX_MONSTER_IN_MONSTER_SIGHT];
	
	for (i = 0; i < data->cur_sight_monster; ++i)
	{
		monster_struct *monster = monster_manager::get_monster_by_id(data->sight_monster[i]);
		if (!monster)
		{
				//在视野里面居然找不到？  bug ？
			LOG_ERR("%s %d: %lu can not find sight monster %lu", __FUNCTION__, __LINE__, data->player_id, data->sight_monster[i]);
			continue;
		}
		for (j = 0; j < n_del; ++j)
		{
			if (monster->area != del_area[j])
				continue;
			del_sight_monster[index++] = monster;
		}
	}

	for (i = 0; i < index; ++i)
		del_monster_from_sight_both(del_sight_monster[i]);
	
}
void monster_struct::del_sight_partner_in_area(int n_del, area_struct **del_area)
{
	int i, j;
	int index = 0;
	partner_struct *del_sight_partner[MAX_PARTNER_IN_MONSTER_SIGHT];
	
	for (i = 0; i < data->cur_sight_partner; ++i)
	{
		partner_struct *partner = partner_manager::get_partner_by_uuid(data->sight_partner[i]);
		if (!partner)
		{
				//在视野里面居然找不到？  bug ？
			LOG_ERR("%s %d: %lu can not find sight partner %lu", __FUNCTION__, __LINE__, data->player_id, data->sight_partner[i]);
			continue;
		}
		for (j = 0; j < n_del; ++j)
		{
			if (partner->area != del_area[j])
				continue;
			del_sight_partner[index++] = partner;
		}
	}

	for (i = 0; i < index; ++i)
		del_partner_from_sight_both(del_sight_partner[i]);
	
}

void monster_struct::update_sight(area_struct *old_area, area_struct *new_area)
{
	if (!old_area || !new_area || old_area == new_area)
		return;
	area_struct *del_area[MAX_NEIGHBOUR_AREA + 1];
	area_struct *add_area[MAX_NEIGHBOUR_AREA + 1];
	int n_add, n_del;
	area_struct::area_neighbour_diff(old_area, new_area, del_area, &n_del, add_area, &n_add);
	assert(n_add <= MAX_NEIGHBOUR_AREA + 1);
	assert(n_del <= MAX_NEIGHBOUR_AREA + 1);		

	int delete_player_id_index = 0;
	uint64_t delete_player_id[MAX_PLAYER_IN_PLAYER_SIGHT];
//	int add_player_id_index = 0;
//	uint64_t add_player_id[MAX_PLAYER_IN_PLAYER_SIGHT];
	
	SightChangedNotify notify;
	sight_changed_notify__init(&notify);

	del_sight_player_in_area(n_del, &del_area[0], &delete_player_id_index, &delete_player_id[0]);
	del_sight_monster_in_area(n_del, &del_area[0]);
	del_sight_truck_in_area(n_del, &del_area[0]);
	del_sight_partner_in_area(n_del, &del_area[0]);	

		//发送给需要在视野里面删除怪物的通知
	notify.n_delete_monster = 1;
	uint64_t player_ids[1];
	player_ids[0] = data->player_id;
	notify.delete_monster = player_ids;
	uint64_t *ppp = conn_node_gamesrv::prepare_broadcast_msg_to_players(MSG_ID_SIGHT_CHANGED_NOTIFY, &notify, (pack_func)sight_changed_notify__pack);
	for (int i = 0; i < delete_player_id_index; ++i)
		conn_node_gamesrv::broadcast_msg_add_players(delete_player_id[i], ppp);
	conn_node_gamesrv::broadcast_msg_send();

	sight_changed_notify__init(&notify);
	uint16_t add_player_id_index = 0;
	for (int i = 0; i < n_add; ++i)
	{
		add_area_player_to_sight(add_area[i], &add_player_id_index, delete_player_id);
		add_area_monster_to_sight(add_area[i]);
		add_area_truck_to_sight(add_area[i]);
		add_area_partner_to_sight(add_area[i]);				
	}
	if (add_player_id_index > 0)
	{
		sight_changed_notify__init(&notify);		
		SightMonsterInfo my_player_info[1];
		SightMonsterInfo *my_player_info_point[1];	
		my_player_info_point[0] = &my_player_info[0];
		notify.add_monster = my_player_info_point;
		pack_sight_monster_info(my_player_info_point[0]);
			//发送给需要在视野里面添加玩家的通知
		notify.n_add_monster = 1;
		
		ppp = conn_node_gamesrv::prepare_broadcast_msg_to_players(MSG_ID_SIGHT_CHANGED_NOTIFY, &notify, (pack_func)sight_changed_notify__pack);
		for (int i = 0; i < add_player_id_index; ++i)
			conn_node_gamesrv::broadcast_msg_add_players(delete_player_id[i], ppp);
		conn_node_gamesrv::broadcast_msg_send();
	}

	reset_pools();

//	LOG_DEBUG("%s %d: monster[%lu] del player[%u] add player[%u] area from %d to %d", __FUNCTION__, __LINE__,
//		data->player_id, delete_player_id_index, add_player_id_index,
//		old_area - scene->m_area, new_area - scene->m_area);
	
	if (old_area->del_monster_from_area(data->player_id) != 0)
	{
		LOG_ERR("%s %d: can not del monster[%lu] from area[%ld %p]", __FUNCTION__, __LINE__, data->player_id, old_area - scene->m_area, old_area);		
	}
	new_area->add_monster_to_area(data->player_id);
	area = new_area;
}

bool monster_struct::on_player_leave_sight(uint64_t player_id)
{
//	LOG_DEBUG("%s player[%lu] monster[%p][%lu]", __FUNCTION__, player_id, this, get_uuid());	
	if (ai && ai->on_player_leave_sight)
	{
		player_struct *player = player_manager::get_player_by_id(player_id);
		if (player)
			return ai->on_player_leave_sight(this, player);
	}
	if (target && target->is_avaliable() && target->get_uuid() == player_id)		
//	if (target && target->get_uuid() == player_id)
		target = NULL;
	
	return true;
}
bool monster_struct::on_player_enter_sight(uint64_t player_id)
{
//	LOG_DEBUG("%s player[%lu] monster[%p][%lu]", __FUNCTION__, player_id, this, get_uuid());
	return true;
}

bool monster_struct::on_monster_leave_sight(uint64_t uuid)
{
//	if (ai_type == AI_TYPE_NORMAL)
	{
		if (target && target->is_avaliable() && target->get_uuid() == uuid)		
			target = NULL;
	}
	return true;
}
bool monster_struct::on_monster_enter_sight(uint64_t uuid)
{
	return true;
}

bool monster_struct::on_truck_leave_sight(uint64_t uuid)
{
	if (target && target->is_avaliable() && target->get_uuid() == uuid)		
		target = NULL;
	return true;
}
bool monster_struct::on_truck_enter_sight(uint64_t uuid)
{
	return true;
}

bool monster_struct::on_partner_leave_sight(uint64_t uuid)
{
//	if (ai_type == AI_TYPE_NORMAL)
	{
		if (target && target->get_unit_type() == UNIT_TYPE_PARTNER && target->is_avaliable() && target->get_uuid() == uuid)
			target = NULL;
	}
	return true;
}
bool monster_struct::on_partner_enter_sight(uint64_t uuid)
{
	return true;
}

void monster_struct::go_back()
{
		//既然不再使用goback状态，为了避免反复在移动，追击，返回之间切换，设定2秒以上的定时
	uint64_t t = time_helper::get_cached_time() + 2000;
	if (data->ontick_time < t)
		data->ontick_time = t;
	ai_state = AI_PATROL_STATE;
	
	if (ai && ai->on_monster_ai_do_goback)
		return ai->on_monster_ai_do_goback(this);
	
	if (!create_config)
		return;
	on_go_back();
	reset_pos();
	// if(ai_type == 22)
	// {
	// 	data->move_path.pos[1].pos_x = ai_data.circle_ai.ret_pos.pos_x;
	// 	data->move_path.pos[1].pos_z = ai_data.circle_ai.ret_pos.pos_z;

	// }
	// else
	{
		data->move_path.pos[1].pos_x = get_born_pos_x();
		data->move_path.pos[1].pos_z = get_born_pos_z();
	}
//		ai_state = AI_GO_BACK_STATE;
	send_patrol_move();
}

// int monster_struct::count_rect_unit_at_pos(struct position *start_pos, std::vector<unit_struct *> *ret, uint max, double length, double width)
// {
// 	double angle = data->angle;//pos_to_angle(my_pos->pos_x, my_pos->pos_z);
// 	double cos = qFastCos(angle);
// 	double sin = qFastSin(angle);
// //	LOG_DEBUG("jacktang sin = %.2f cos = %.2f", sin, cos);
// //	double point_x1 = cos*(my_pos->pos_x)-sin*(my_pos->pos_z);
// //	double point_z1 = cos*(my_pos->pos_z)+sin*(my_pos->pos_x);
// 	double x1, x2;
// 	double z1, z2;
// //	x1 = point_x1;
// //	x2 = point_x1 + length;
// //	z1 = point_z1 - width;
// //	z2 = point_z1 + width;

// 	x1 = 0;	
// 	x2 = x1 + length;
// //	z1 = -width / 2;
// 	z2 = width / 2;
// 	z1 = -z2;
	
// 	for (int i = 0; i < data->cur_sight_player; ++i)
// 	{
// 		player_struct *player = player_manager::get_player_by_id(data->sight_player[i]);
// 		if (!player || !player->is_alive())
// 		{
// 			LOG_ERR("%s %d: player[%lu] in sight", __FUNCTION__, __LINE__, data->sight_player[i]);
// 			continue;
// 		}
// 		struct position *pos = player->get_pos();

// 		double pos_x = pos->pos_x - start_pos->pos_x;
// 		double pos_z = pos->pos_z - start_pos->pos_z;
// 		double target_x1 = cos*(pos_x)-sin*(pos_z);
// 		double target_z1 = cos*(pos_z)+sin*(pos_x);		
		
// //		double target_x1 = cos*(pos->pos_x)-sin*(pos->pos_z);
// //		double target_z1 = cos*(pos->pos_z)+sin*(pos->pos_x);

// 		if (target_x1 >= x1 && target_x1 <= x2 && target_z1 >= z1 && target_z1 <= z2)
// 		{
// //			LOG_DEBUG("%s:  hit: angle[%.2f] x1[%.2f] x2[%.2f] x[%.2f] z1[%.2f] z2[%.2f] z[%.2f]", "jacktang", angle, x1, x2, target_x1, z1, z2, target_z1);					
// 			ret->push_back(player);
// 			if (ret->size() >= max)
// 				return (0);
// 		}
// //		else
// //		{
// //			LOG_DEBUG("%s: miss: angle[%.2f] x1[%.2f] x2[%.2f] x[%.2f] z1[%.2f] z2[%.2f] z[%.2f]", "jacktang", angle, x1, x2, target_x1, z1, z2, target_z1);		
// //		}
// 	}	
// 	return (0);
// }

// int monster_struct::count_rect_unit(std::vector<unit_struct *> *ret, uint max, double length, double width)
// {
// 	struct position *my_pos = get_pos();
// 	return count_rect_unit_at_pos(my_pos, ret, max, length, width);
	
// }
// int monster_struct::count_circle_unit(std::vector<unit_struct *> *ret, uint max, double radius)
// {
// 	struct position *my_pos = get_pos();	
// 	radius = radius * radius;
// 	for (int i = 0; i < data->cur_sight_player; ++i)
// 	{
// 		player_struct *player = player_manager::get_player_by_id(data->sight_player[i]);
// 		if (!player || !player->is_alive())
// 		{
// 			LOG_ERR("%s %d: player[%lu] in sight", __FUNCTION__, __LINE__, data->sight_player[i]);
// 			continue;
// 		}
// 		double x = my_pos->pos_x - player->get_pos()->pos_x;
// 		double z = my_pos->pos_z - player->get_pos()->pos_z;
// 		if (x * x + z * z > radius)
// 			continue;
// 		ret->push_back(player);
// 		if (ret->size() >= max)
// 			return (0);		
// 	}
// 	return (0);	
// }
// int monster_struct::count_fan_unit(std::vector<unit_struct *> *ret, uint max, double radius, double angle)
// {
// 	struct position *my_pos = get_pos();	
// 	radius = radius * radius;
// 	double my_angle = pos_to_angle(my_pos->pos_x, my_pos->pos_z);
// 	double angle_min = my_angle - angle;
// 	double angle_max = my_angle + angle;
	
// 	for (int i = 0; i < data->cur_sight_player; ++i)
// 	{
// 		player_struct *player = player_manager::get_player_by_id(data->sight_player[i]);
// 		if (!player || !player->is_alive())
// 		{
// 			LOG_ERR("%s %d: player[%lu] in sight", __FUNCTION__, __LINE__, data->sight_player[i]);
// 			continue;
// 		}
// 		double x = my_pos->pos_x - player->get_pos()->pos_x;
// 		double z = my_pos->pos_z - player->get_pos()->pos_z;
// 		if (x * x + z * z > radius)
// 			continue;
// 		double angle_target = pos_to_angle(player->get_pos()->pos_x, player->get_pos()->pos_z);
// 		if (angle_target >= angle_min && angle_target <= angle_max)
// 		{
// 			ret->push_back(player);
// 			if (ret->size() >= max)
// 				return (0);
// 		}
// 	}
// 	return (0);	
// }

uint64_t monster_struct::count_rand_patrol_time()
{
	if (ai_config->StopMax <= ai_config->StopMin)
		return ai_config->StopMin;
	
	return random() % (ai_config->StopMax - ai_config->StopMin) + ai_config->StopMin;
}

int monster_struct::count_skill_hit_unit(std::vector<unit_struct *> *ret, struct SkillTable *config, unit_struct *target)
{
//	struct SkillTable *config = get_config_by_id(data->skill_id, &skill_config);
	if (!config)
		return (-1);
	switch (config->RangeType)
	{
		case SKILL_RANGE_TYPE_RECT:
			return count_rect_unit(data->angle, ret, config->MaxCount, config->Radius, config->Angle);
		case SKILL_RANGE_TYPE_CIRCLE:
			return count_circle_unit(ret, config->MaxCount, config->Radius);			
		case SKILL_RANGE_TYPE_FAN:
			return count_fan_unit(ret, config->MaxCount, config->Radius, config->Angle);
		case SKILL_RANGE_TYPE_TARGET_RECT:
		{
			struct position *pos = &data->skill_target_pos;
			return count_rect_unit_at_pos(data->angle, pos, ret, config->MaxCount, config->Radius, config->Angle);
		}
		default:
			return -10;
	}
	
	return (0);
}

void monster_struct::send_patrol_move()
{
	data->move_path.start_time = time_helper::get_cached_time();
	data->move_path.max_pos = 1;
	data->move_path.cur_pos = 0;
	broadcast_monster_move();
}

void monster_struct::do_taunt_action()
{
	if (is_unit_in_move())
		return;
	
	unit_struct *target = get_taunt_target();
	if (!target || !target->is_avaliable()
		|| !target->is_alive())
		return;
	struct position *my_pos = get_pos();
	struct position *his_pos = target->get_pos();

	data->skill_id = 0;
	if (config->n_Skill <= 0)
		return;
	uint32_t skill_id = config->Skill[0];
	if (skill_id == 0)
		return;
	struct SkillTable *config = get_config_by_id(skill_id, &skill_config);
	if (config == NULL)
	{
		return;
	}
	if (!check_distance_in_range(my_pos, his_pos, config->SkillRange))
	{
		reset_pos();
		data->move_path.pos[1].pos_x = his_pos->pos_x;
		data->move_path.pos[1].pos_z = his_pos->pos_z;
		send_patrol_move();
	}
	else
	{
		cast_immediate_skill_to_player(skill_id, target);
	}
}

void monster_struct::clear_cur_skill()
{
	data->skill_id = 0;
}

void monster_struct::cast_immediate_skill_to_player(uint64_t skill_id, unit_struct *player)
{
	player_struct *owner = NULL;
	if (data->owner)
		owner = player_manager::get_player_by_id(data->owner);		
	return monster_cast_immediate_skill_to_player(skill_id, this, owner, player);

	
// //	if (!check_can_attack(this, player))
// 	if (get_unit_fight_type(this, player) != UNIT_FIGHT_TYPE_ENEMY)
// 	{
// 		return;
// 	}

// //	if (player->buff_state & BUFF_STATE_GOD)
// //	{
// //		return;
// //	}
	
// 	int n_hit_effect = 0;
// 	int n_buff = 0;
// 	assert(player && player->is_avaliable());
// 	cached_hit_effect_point[n_hit_effect] = &cached_hit_effect[n_hit_effect];
// 	skill_hit_effect__init(&cached_hit_effect[n_hit_effect]);
// 	uint32_t add_num = 0;
// 	int32_t damage = 0;

// 	struct SkillLvTable *lv_config1, *lv_config2;
// 	struct PassiveSkillTable *pas_config;
// 	struct SkillTable *ski_config;
// 	get_skill_configs(1, skill_id, &ski_config, &lv_config1, &pas_config, &lv_config2);
// 	if (!lv_config1 && !lv_config2)
// 	{
// 		LOG_ERR("%s %d: skill[%lu] no config", __FUNCTION__, __LINE__, skill_id);
// 		return;
// 	}

// 	int32_t other_rate = count_other_skill_damage_effect(this, player);							
// 	damage += count_skill_total_damage(UNIT_FIGHT_TYPE_ENEMY, ski_config, lv_config1,
// 		pas_config, lv_config2,
// 		this, player,
// 		&cached_hit_effect[n_hit_effect].effect,
// 		&cached_buff_id[n_buff],
// 		&add_num, other_rate);

// 	player->on_hp_changed();

// 	if (player->get_unit_type() == UNIT_TYPE_PLAYER)
// 	{
// 		player_struct *t = ((player_struct *)player);
// 		raid_struct *raid = t->get_raid();
// 		if (raid)
// 			raid->on_monster_attack(this, t, damage);
// 	}

// 	LOG_DEBUG("%s: unit[%lu][%p] damage[%d] hp[%f]", __FUNCTION__, player->get_uuid(), player, damage, player->get_attr(PLAYER_ATTR_HP));

// 	uint32_t life_steal = count_life_steal_effect(damage);
// 	uint32_t damage_return = count_damage_return(damage, player);

// 	on_hp_changed();

// 	PosData attack_pos;
// 	pos_data__init(&attack_pos);
// 	attack_pos.pos_x = get_pos()->pos_x;
// 	attack_pos.pos_z = get_pos()->pos_z;
	
// 	cached_hit_effect[n_hit_effect].playerid = player->get_uuid();
// 	cached_hit_effect[n_hit_effect].n_add_buff = add_num;
// 	cached_hit_effect[n_hit_effect].add_buff = &cached_buff_id[n_buff];
// 	cached_hit_effect[n_hit_effect].hp_delta = damage;
// 	cached_hit_effect[n_hit_effect].cur_hp = player->get_attr(PLAYER_ATTR_HP);
// //	cached_hit_effect.attack_pos = &attack_pos;
// 	cached_hit_effect[n_hit_effect].target_pos = &cached_target_pos[n_hit_effect];	
// //	pos_data__init(&cached_attack_pos[n_hit_effect]);
// 	pos_data__init(&cached_target_pos[n_hit_effect]);	
// //	cached_attack_pos[n_hit_effect].pos_x = monster->get_pos()->pos_x;
// //	cached_attack_pos[n_hit_effect].pos_z = monster->get_pos()->pos_z;
// 	cached_target_pos[n_hit_effect].pos_x = player->get_pos()->pos_x;
// 	cached_target_pos[n_hit_effect].pos_z = player->get_pos()->pos_z;			
	
// 	n_buff += add_num;
// 	++n_hit_effect;

// 	SkillHitImmediateNotify notify;
// 	skill_hit_immediate_notify__init(&notify);
// 	notify.playerid = data->player_id;

// 	player_struct *owner = NULL;
// 	if (data->owner)
// 		owner = player_manager::get_player_by_id(data->owner);		

// 	if (owner)
// 	{
// 		notify.owneriid = data->owner;
// 		notify.ownername = owner->get_name();
// 	}
// 	else
// 	{
// 		notify.owneriid = data->player_id;
// 	}
	
// 	notify.skillid = skill_id;
// 	notify.target_player = cached_hit_effect_point[0];
// 	notify.attack_pos = &attack_pos;
// 	notify.attack_cur_hp = get_attr(PLAYER_ATTR_HP);
// 	notify.life_steal = life_steal;
// 	notify.damage_return = damage_return;
// 	player->broadcast_to_sight(MSG_ID_SKILL_HIT_IMMEDIATE_NOTIFY, &notify, (pack_func)skill_hit_immediate_notify__pack, true);
	
// 	if (!player->is_alive())
// 	{
// 		player->on_dead(player);
// 	}
// 	else
// 	{
// 		player->on_beattack(this, skill_id, damage);
// 	}
// 	if (!is_alive())
// 	{
// 		on_dead(player);
// 	}
}

bool monster_struct::try_active_attack()
{
	if (ai && ai->monster_ai_choose_target)
	{
		target = ai->monster_ai_choose_target(this);
		return (target != NULL);
	}
	
	assert(ai_config);
	if (ai_config->ActiveAttackRange == 0)
		return false;
	double range = ai_config->ActiveAttackRange * ai_config->ActiveAttackRange;
	struct position *my_pos = get_pos();
	for (int i = 0; i < data->cur_sight_truck; ++i)
	{
		cash_truck_struct *truck = cash_truck_manager::get_cash_truck_by_id(data->sight_truck[i]);
		if (!truck)
			continue;
		if (get_unit_fight_type(this, truck) != UNIT_FIGHT_TYPE_ENEMY)							
			continue;
		
		struct position *pos = truck->get_pos();
		double x = pos->pos_x - my_pos->pos_x;
		double z = pos->pos_z - my_pos->pos_z;
		if (x * x + z * z > range)
			continue;
		target = truck;
		return true;
	}
	for (int i = 0; i < data->cur_sight_player; ++i)
	{
		player_struct *player = player_manager::get_player_by_id(data->sight_player[i]);
		if (!player)
			continue;
		if (get_unit_fight_type(this, player) != UNIT_FIGHT_TYPE_ENEMY)							
			continue;
		
		struct position *pos = player->get_pos();
		double x = pos->pos_x - my_pos->pos_x;
		double z = pos->pos_z - my_pos->pos_z;
		if (x * x + z * z > range)
			continue;
		target = player;
		return true;
	}
	for (int i = 0; i < data->cur_sight_monster; ++i)
	{
		monster_struct *target_monster = monster_manager::get_monster_by_id(data->sight_monster[i]);
		if (!target_monster)
			continue;

		if (get_unit_fight_type(this, target_monster) != UNIT_FIGHT_TYPE_ENEMY)									
			continue;
		
		struct position *pos = target_monster->get_pos();
		double x = pos->pos_x - my_pos->pos_x;
		double z = pos->pos_z - my_pos->pos_z;
		if (x * x + z * z > range)
			continue;
		target = target_monster;
		return true;
	}
	return false;
}

void monster_struct::monster_dead_creat_collect(unit_struct *murderer)
{
	if(murderer == NULL)
	{
		LOG_ERR("[%s:%d] monster dead creat collect fail", __FUNCTION__, __LINE__);
		return;
	}

	player_struct *player = NULL;

	if(murderer->get_unit_type() == UNIT_TYPE_PLAYER)
	{
		player = (player_struct*)murderer;
	}
	else if(murderer->get_unit_type() == UNIT_TYPE_PARTNER)
	{
	
		player = ((partner_struct*)murderer)->m_owner;
	}
	else
	{
		return;
	}

	if(player == NULL)
	{		
		LOG_ERR("[%s]:[%d] get player failed", __FUNCTION__, __LINE__);
		return;
	}
	if(this->scene == NULL)
	{		
		LOG_ERR("[%s]:[%d] get scene failed", __FUNCTION__, __LINE__);
		return;
	}

	if(this->config == NULL)
	{
		return;
	}

	uint64_t baseAttribute = this->config->BaseAttribute;
	uint64_t id = baseAttribute * 1000 + this->get_attr(PLAYER_ATTR_LEVEL); 
	ActorAttributeTable* actor_config = get_config_by_id(id, &actor_attribute_config);
	if(actor_config == NULL)
	{
		LOG_ERR("[%s]:[%d] get ActorAttributeTable failed", __FUNCTION__, __LINE__);
		return;
	}

	if(actor_config->CollectionDrop == 0)
	{
		return;
	}

	uint64_t drop_gailv = rand() % 10000 + 1;

	if(drop_gailv > actor_config->CollectionProbability)
	{
		return;
	}

	Collect::CreateCollectByPos(this->scene, actor_config->CollectionDrop, this->get_pos()->pos_x , 10000, this->get_pos()->pos_z, 0, player);

}

void monster_struct::world_boss_refresf_player_redis_info(unit_struct *murderer, double befor_hp, int32_t damage)
{

	//世界boss被击，在活动开启期间特殊处理
	if(befor_hp > get_attr(PLAYER_ATTR_MAXHP) || damage <= 0)
		return;
	if(murderer == NULL)
		return;
	if(this->config == NULL)
		return;
	if(this->config->Type != 5)
		return;
	uint32_t cd =0;
	if(!check_active_open(WORD_BOSS_ACTIVE_ID, cd))
		return;
	std::map<uint64_t, struct WorldBossTable *>::iterator p = monster_to_world_boss_config.find(data->monster_id);
	if(p == monster_to_world_boss_config.end())
		return;
	player_struct *player = NULL;
	if(murderer->get_unit_type() == UNIT_TYPE_PLAYER)
	{
		player = (player_struct*)murderer;
	}
	else if(murderer->get_unit_type() == UNIT_TYPE_PARTNER)
	{
	
		murderer = ((partner_struct*)murderer)->m_owner;
	}
	else
	{
		return;
	}
	if(player == NULL || player->data == NULL)
		return;

	PlayerWorldBossRedisinfo info;
	player_world_boss_redisinfo__init(&info);

	double real_damage = befor_hp > damage ? damage:befor_hp; 

	info.name  = player->data->name;
	if(player->get_level() > p->second->RewardLevel)
	{
		info.score = 0;
	}
	else
	{
		info.score = ceil(real_damage *  p->second->Coefficient);
	}
	info.boss_id = p->second->ID;
	info.cur_hp = get_attr(PLAYER_ATTR_HP) > 0 ? get_attr(PLAYER_ATTR_HP):0;
	info.max_hp = get_attr(PLAYER_ATTR_MAXHP);


	EXTERN_DATA extern_data;
	extern_data.player_id = player->data->player_id;

	fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, SERVER_PROTO_WORLDBOSS_PLAYER_REDIS_INFO, player_world_boss_redisinfo__pack, info);
	
   //如果世界boss死了发公告
   if(get_attr(PLAYER_ATTR_HP) <= 0)
   {
	   NoticeTable *table = get_config_by_id(330510009, &notify_config);
	   if(table == NULL)
		   return;
	   SceneResTable *scen_config = get_config_by_id(p->second->SceneID,&scene_res_config);
	   if(scen_config == NULL)
		   return;
		
	   char buff[512];
	   ChatHorse send;
	   chat_horse__init(&send);
	   send.id = 330510009;
	   send.prior = table->Priority;
	   send.content = buff;
	   uint32_t c[MAX_CHANNEL] = { 1,2,3,4,5,6 };
	   for (uint32_t i = 0; i < table->n_NoticeChannel; ++i)
	   {
		   c[i] = table->NoticeChannel[i];
	   }
	   send.channel = c;
	   send.n_channel = table->n_NoticeChannel;

	   snprintf(buff, 510, table->NoticeTxt, scen_config->SceneName,p->second->Name,player->get_name());
	   conn_node_gamesrv::send_to_all_player(MSG_ID_CHAT_HORSE_NOTIFY, &send, (pack_func)chat_horse__pack);
   }	   
}

bool monster_struct::on_unit_leave_sight(uint64_t uuid)
{
	if (config->HateType != MONSTER_HATETYPE_DEFINE_BOSS)
		return true;
	
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

void monster_struct::update_target()
{
	if (config->HateType != MONSTER_HATETYPE_DEFINE_BOSS)
		return;
	
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

	LOG_DEBUG("%s: monster %lu set target = %lu[%d]", __FUNCTION__, get_uuid(), target_uuid, max_hate);
	
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

void monster_struct::count_hate(unit_struct *player, uint32_t skill_id, int32_t damage)
{
	if (config->HateType != MONSTER_HATETYPE_DEFINE_BOSS)
		return;
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
