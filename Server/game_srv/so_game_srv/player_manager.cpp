#include "game_event.h"
#include "so_game_srv/partner_manager.h"
#include "player_manager.h"
#include "scene_manager.h"
#include "scene.h"
#include "global_param.h"
#include "camp_judge.h"
#include "skill_manager.h"
#include "mem_pool.h"
#include "uuid.h"
#include "game_config.h"
#include "lua_config.h"
#include "conn_node_gamesrv.h"
#include "cash_truck_manager.h"
#include <stdio.h>
#include <errno.h>

player_manager::player_manager()
{
}

player_manager::~player_manager()
{
}


/////////////////////  以下是静态成员
ai_player_handle player_manager::m_ai_player_handle[MAX_PLAYER_AI_TYPE];
// std::map<uint64_t, player_struct *> player_manager::player_manager_all_ai_players_id;
// std::map<uint64_t, player_struct *> player_manager::player_manager_all_players_id;
// std::list<player_struct *> player_manager::player_manager_player_free_list;
// std::set<player_struct *> player_manager::player_manager_player_used_list;		
// comm_pool player_manager::player_manager_player_data_pool;
int player_manager::add_player(player_struct *p)
{
	player_manager_all_players_id[p->data->player_id] = p;
	return (0);
}

int player_manager::remove_player(player_struct *p)
{
	player_manager_all_players_id.erase(p->data->player_id);	
	return (0);
}

player_struct * player_manager::get_player_by_id(uint64_t id)
{
	std::map<uint64_t, player_struct *>::iterator it = player_manager_all_players_id.find(id);
	if (it != player_manager_all_players_id.end())
		return it->second;
	return NULL;	
}

player_struct * player_manager::get_ai_player_by_id(uint64_t id)
{
	std::map<uint64_t, player_struct *>::iterator it = player_manager_all_ai_players_id.find(id);
	if (it != player_manager_all_ai_players_id.end())
		return it->second;
	return NULL;
}

player_struct *player_manager::get_online_player(uint64_t id)
{
	std::map<uint64_t, player_struct *>::iterator it = player_manager_all_players_id.find(id);
	if (it == player_manager_all_players_id.end())
		return NULL;
	if (!it->second->is_online())
		return NULL;
	return it->second;
}

int player_manager::send_to_player(uint64_t player_id, PROTO_HEAD *head)
{
	EXTERN_DATA extern_data;
	extern_data.player_id = player_id;
	conn_node_base::add_extern_data(head, &extern_data);
	if (conn_node_gamesrv::connecter.send_one_msg((PROTO_HEAD *)head, 1) != (int)(ENDION_FUNC_4(head->len))) {
		LOG_ERR("%s %d: send to all failed err[%d]", __FUNCTION__, __LINE__, errno);
		return (-1);
	}
	return (0);
}

unsigned int player_manager::get_pool_max_num()
{
	return player_manager_player_data_pool.num;
}

int player_manager::init_player_struct(int num, unsigned long key)
{
	player_struct *player;
	for (int i = 0; i < num; ++i) {
		player = new player_struct();
		player_manager_player_free_list.push_back(player);
#ifdef __RAID_SRV__
		player->data = (player_data *)malloc(sizeof(player_data));
#endif		
	}
#ifdef __RAID_SRV__
	return (0);
#else	
	LOG_DEBUG("%s: init mem[%lu][%lu]", __FUNCTION__, sizeof(player_struct) * num, sizeof(player_data) * num);
	return init_comm_pool(0, sizeof(player_data), num, key, &player_manager_player_data_pool);
#endif			
}

int player_manager::resume_player_struct(int num, unsigned long key)
{
	player_struct *player;
	for (int i = 0; i < num; ++i) {
		player = new player_struct();
		player_manager_player_free_list.push_back(player);
	}

	int ret = init_comm_pool(1, sizeof(player_data), num, key, &player_manager_player_data_pool);
	int index = 0;
	for (;;) {
		struct player_data *data = (struct player_data *)get_next_inuse_comm_pool_entry(&player_manager_player_data_pool, &index);
		if (!data)
			break;
		LOG_DEBUG("%s %d: status[%d], player_id[%lu], name[%s]\n",
			__FUNCTION__, __LINE__, data->status, data->player_id, data->name);
		player = player_manager_player_free_list.back();
		if (!player) {
			LOG_ERR("%s %d: get free player failed", __FUNCTION__, __LINE__);
			return (-1);
		}
		player_manager_player_free_list.pop_back();	
		player->data = data;
		add_player(player);
	}
	return (ret);
}
/*
int player_manager::resume_player_bag_data(player_struct *player)
{
	for (int i = 0; i < player->data->active_equip_bag_grids; ++i) {
		if (player->data->equip_grids[i].id == 0 || player->data->equip_grids[i].uuid == 0)
			continue;
		equip_struct *equip = equip_manager::get_equip_by_id(player->data->equip_grids[i].uuid);
		if (!equip) {
			LOG_ERR("%s %d: resume equip %lu failed", __FUNCTION__, __LINE__, player->data->equip_grids[i].uuid);
			return (-1);
		}
		player->data->equip_grids[i].equip = equip;
	}
	for (int i = 0; i < EQUIPPOSDEFINE_END; ++i) {
		if (player->data->used_equip_grids[i].id == 0 || player->data->used_equip_grids[i].uuid == 0)
			continue;
		equip_struct *equip = equip_manager::get_equip_by_id(player->data->used_equip_grids[i].uuid);
		if (!equip) {
			LOG_ERR("%s %d: resume equip %lu failed", __FUNCTION__, __LINE__, player->data->used_equip_grids[i].uuid);
			return (-1);
		}
		player->data->used_equip_grids[i].equip = equip;
	}
	return (0);
}
*/
player_struct *player_manager::alloc_player()
{
	player_struct *ret = NULL;
	player_data *data = NULL;
	if (player_manager_player_free_list.empty())
		return NULL;
	ret = player_manager_player_free_list.back();
	player_manager_player_free_list.pop_back();

#ifdef __RAID_SRV__
	if (!ret)
		goto fail;	
	memset(ret->data, 0, sizeof(player_data));	
#else	
	data = (player_data *)comm_pool_alloc(&player_manager_player_data_pool);
	if (!data)
		goto fail;
	memset(data, 0, sizeof(player_data));
	ret->data = data;
#endif	
	player_manager_player_used_list.insert(ret);
	ret->init_player();

	LOG_DEBUG("[%s:%d] player_id[%lu], player:%p, data:%p", __FUNCTION__, __LINE__, ret->data->player_id, ret, ret->data);	
	return ret;
fail:
	if (ret) {
		player_manager_player_used_list.erase(ret);
		player_manager_player_free_list.push_back(ret);
	}
	if (data) {
		comm_pool_free(&player_manager_player_data_pool, data);
	}
	return NULL;
}

void player_manager::delete_player_by_id(uint64_t player_id)
{
	player_struct *p = get_player_by_id(player_id);
	if (p)
		delete_player(p);
}

void player_manager::delete_player(player_struct *p)
{
//	LOG_DEBUG("%s %d: player_id[%lu]", __FUNCTION__, __LINE__, p->data->player_id);
	
	player_manager_player_used_list.erase(p);
	player_manager_player_free_list.push_back(p);

	p->clear();
	p->interrupt();
	
	LOG_DEBUG("[%s:%d] player_id[%lu], player:%p, data:%p", __FUNCTION__, __LINE__, p->data->player_id, p, p->data);

	if (get_entity_type(p->data->player_id) == ENTITY_TYPE_AI_PLAYER)
		player_manager_all_ai_players_id.erase(p->data->player_id);

	if (p->data) {
		remove_player(p);
#ifdef __RAID_SRV__
#else		
		comm_pool_free(&player_manager_player_data_pool, p->data);		
		p->data = NULL;
#endif		
	}
}

void player_manager::on_tick_10()
{
	for (std::set<player_struct *>::iterator iter = player_manager_player_used_list.begin(); iter != player_manager_player_used_list.end(); ++iter)
	{
		(*iter)->on_tick_10();
	}
}

void player_manager::on_tick_5()
{
#ifndef USE_AISRV	
	for (std::map<uint64_t, player_struct *>::iterator iter = player_manager_all_ai_players_id.begin(); iter != player_manager_all_ai_players_id.end(); ++iter)
	{
		player_struct *player = iter->second;
		if (!player->scene)
			continue;
		if (!player->ai_data)
			continue;
		if (player->ai_data->player_ai_index > 0
			&& player->ai_data->player_ai_index < MAX_PLAYER_AI_TYPE
			&& m_ai_player_handle[player->ai_data->player_ai_index])
			m_ai_player_handle[player->ai_data->player_ai_index](player);
	}
#endif	
}

void player_manager::on_tick_1()
{
	for (std::set<player_struct *>::iterator iter = player_manager_player_used_list.begin(); iter != player_manager_player_used_list.end(); ++iter)
	{
		(*iter)->on_tick();
	}
}

player_struct *player_manager::add_player(uint64_t player_id)
{
	player_struct *ret = alloc_player();
	if (!ret)
		return NULL;
	ret->data->player_id = player_id;
	add_player(ret);
	return ret;
}

char *player_manager::get_rand_player_name(int index)
{
	int size = rand_name_config.size();
//	int rand = random() % size;
	return rand_name_config[index % size]->Name;
}

player_struct * player_manager::create_doufachang_ai_player(DOUFACHANG_LOAD_PLAYER_ANSWER *ans)
{
	player_struct *ret;
	uint64_t player_id = alloc_ai_player_uuid();	
	
//	scene_struct *scene = NULL;
//	bool can_delete_when_fail = true;

	ret = add_player(player_id);
	if (!ret) {
		LOG_ERR("%s %d: add player[%lu] fail", __FUNCTION__, __LINE__, player_id);		
		return NULL;
	}

	LOG_DEBUG("%s %d: player_id[%lu][%lu][%lu] player[%p][%p]", __FUNCTION__, __LINE__, ans->player_id, ans->target_id, player_id, ret, ret->data);
	
	if (ret->unpack_dbinfo_to_playerinfo(ans->data, ans->data_size) != 0) {
		LOG_ERR("%s %d: unpack info[%lu] fail", __FUNCTION__, __LINE__, player_id);
		delete_player(ret);
		return NULL;
	}
	strncpy(ret->data->name, ans->name, MAX_PLAYER_NAME_LEN);
	ret->data->name[MAX_PLAYER_NAME_LEN] = '\0';
	ret->data->attrData[PLAYER_ATTR_LEVEL] = ans->lv<=0?1:ans->lv;
	ret->data->attrData[PLAYER_ATTR_JOB] = ans->job;

	if (ret->data->attrData[PLAYER_ATTR_BAGUA] == 0)
	{
		ret->data->attrData[PLAYER_ATTR_BAGUA] = 1;
	}
	ret->load_partner_end();
	ret->calculate_attribute();

	ret->data->attrData[PLAYER_ATTR_PK_TYPE] = PK_TYPE_MURDER;
		//ai
	ret->ai_data = create_ai_data();
	if (ret->ai_data)
	{
		ret->ai_data->origin_player_id = ans->target_id;
		ret->ai_data->active_attack_range = sg_doufachang_ai[0];
		ret->ai_data->chase_range = sg_doufachang_ai[1];
		ret->ai_data->player_ai_index = 2;
			//停止AI
		ret->ai_data->stop_ai = true;
	}
	ret->data->attrData[PLAYER_ATTR_HP] = ret->data->attrData[PLAYER_ATTR_MAXHP];
	ret->data->attrData[PLAYER_ATTR_MOVE_SPEED] = 5;

	ret->m_skill.adjust_ai_player_skill();
	
		//登陆成功
	ret->data->status = ONLINE;
	ret->down_horse();

	player_manager_all_ai_players_id[player_id] = ret;
	return ret;
}

player_struct *player_manager::create_doufachang_ai_player(player_struct *player)
{
	player_struct *ret;
	uint64_t player_id = alloc_ai_player_uuid();
	
	ret = add_player(player_id);
	if (!ret) {
		LOG_ERR("%s %d: add player[%lu] fail", __FUNCTION__, __LINE__, player_id);		
		return NULL;
	}

	LOG_DEBUG("%s %d: player_id[%lu] id[%lu] player[%p][%p]", __FUNCTION__, __LINE__, player->get_uuid(), player_id, ret, ret->data);

	strcpy(ret->data->name, player->get_name());
	memcpy(&ret->data->attrData[0], &player->data->attrData[0], sizeof(ret->data->attrData));
	memcpy(&ret->data->buff_fight_attr[0], &player->data->buff_fight_attr[0], sizeof(ret->data->buff_fight_attr));	
	ret->data->attrData[PLAYER_ATTR_PK_TYPE] = PK_TYPE_MURDER;
		//ai
	ret->ai_data = create_ai_data();
	if (ret->ai_data)
	{
		ret->ai_data->origin_player_id = player->get_uuid();
		ret->ai_data->active_attack_range = sg_doufachang_ai[0];
		ret->ai_data->chase_range = sg_doufachang_ai[1];
		ret->ai_data->player_ai_index = 2;
			//停止AI
		ret->ai_data->stop_ai = true;
	}
	ret->m_skill.copy(&player->m_skill);
	ret->m_skill.adjust_ai_player_skill();
	
	ret->data->attrData[PLAYER_ATTR_HP] = ret->data->attrData[PLAYER_ATTR_MAXHP];
	ret->data->attrData[PLAYER_ATTR_MOVE_SPEED] = 5;

	memset(&ret->data->partner_formation[0], 0, sizeof(ret->data->partner_formation));
	memset(&ret->data->partner_battle[0], 0, sizeof(ret->data->partner_battle));

	for (int i = 0; i < MAX_PARTNER_FORMATION_NUM; ++i)	
	{
		partner_struct *src = player->get_partner_by_uuid(player->data->partner_formation[i]);
		if (src == NULL || src->data == NULL)
		{
			continue;
		}
		partner_struct *partner = partner_manager::create_partner(src->data->partner_id, ret, 0, true);
		if (partner == NULL)
			continue;
		partner->data->attrData[PLAYER_ATTR_MAXHP] = src->data->attrData[PLAYER_ATTR_MAXHP];
		partner->data->attrData[PLAYER_ATTR_HP] = partner->data->attrData[PLAYER_ATTR_MAXHP];
		partner->data->attrData[PLAYER_ATTR_MOVE_SPEED] = src->data->attrData[PLAYER_ATTR_MOVE_SPEED];
		
		memcpy(&partner->data->buff_fight_attr[0], &src->data->buff_fight_attr[0], sizeof(partner->data->buff_fight_attr));
		ret->m_partners.insert(std::make_pair(partner->data->uuid, partner));
		ret->data->partner_formation[i] = partner->data->uuid;
	}
	
		//登陆成功
	ret->data->status = ONLINE;
	ret->down_horse();

	player_manager_all_ai_players_id[player_id] = ret;
	return ret;
}

player_struct * player_manager::create_ai_player(player_struct *player, scene_struct *scene, int name_index, int type)
{
	player_struct *ret;
	uint64_t player_id = alloc_ai_player_uuid();

	type -= 1;
	if (type < 0 || type >= ROBOT_CONFIG_TYPE_SIZE || robot_config[type].size() == 0)
	{
		LOG_ERR("%s: type[%d] wrong", __FUNCTION__, type);
		return NULL;
	}
	
	ret = get_player_by_id(player_id);
	ret = add_player(player_id);
	if (!ret) {
		LOG_ERR("%s %d: add player[%lu] fail", __FUNCTION__, __LINE__, player_id);		
		return NULL;
	}

	LOG_DEBUG("%s %d: player_id[%lu] [%p][%p] ai[%lu] type[%d]", __FUNCTION__, __LINE__, player->get_uuid(),
		ret, ret->data, player_id, type);

	strcpy(ret->data->name, get_rand_player_name(name_index));

	if (scene)
		ret->data->scene_id = scene->m_id;

//	const std::vector<struct ActorRobotTable*> &t = robot_config[type];

	int index = random() % robot_config[type].size();
	struct ActorRobotTable *t = (robot_config[type])[index];

	int fight = player->get_attr(PLAYER_ATTR_FIGHTING_CAPACITY);
//	fight += random() % (sg_pvp_raid_fighting_capacity_range[1] - sg_pvp_raid_fighting_capacity_range[0])
//		+ sg_pvp_raid_fighting_capacity_range[0];

	float rate = random() % (t->FightPro[1] - t->FightPro[0]) + t->FightPro[0];
	rate = rate * 0.01;
	fight = fight * rate;
//	fight = random() % (t->FightPro[1] - t->FightPro[0])
//		+ t->FightPro[0];
	
	if (fight < 100)
		fight = 100;

	int rand_lv = player->get_level() + random() % (TEAM_LEVEL2_DIFF_R - TEAM_LEVEL2_DIFF_L) + TEAM_LEVEL2_DIFF_L;
	if (rand_lv < 1)
		rand_lv = 1;
	else if (rand_lv > 100)
		rand_lv = 100;
	ret->data->attrData[PLAYER_ATTR_LEVEL] = rand_lv;

//	LOG_DEBUG("%s: player_lv[%u] diff_r[%d] diff_l[%d] result[%u]", __FUNCTION__, player->get_level(),
//		TEAM_LEVEL2_DIFF_R, TEAM_LEVEL2_DIFF_L, ret->get_level());
	
	ret->data->attrData[PLAYER_ATTR_FIGHTING_CAPACITY] = fight;
	ret->data->attrData[PLAYER_ATTR_JOB] = t->ID % 16;//index + 1;
	ret->data->attrData[PLAYER_ATTR_HEAD] = t->InitialHead;

		//时装，武器，头饰
	ret->data->attrData[PLAYER_ATTR_WEAPON] = t->WeaponId[random() % t->n_WeaponId];
	ret->data->attrData[PLAYER_ATTR_CLOTHES] = t->ResId[random() % t->n_ResId];
	ret->data->attrData[PLAYER_ATTR_HAT] = t->HairResId[random() % t->n_HairResId];

		//ai
	ret->ai_data = create_ai_data();
	if (ret->ai_data)
	{
		ret->ai_data->active_attack_range = t->ActiveAttackRange;
		ret->ai_data->chase_range = t->ChaseRange;
		ret->ai_data->player_ai_index = 1;
			//停止AI
		ret->ai_data->stop_ai = true;
	}
	
	int max_rate = 0;
	for (size_t i = 0; i < t->n_AttributeType; ++i)
	{
		max_rate += t->AttributePro[i];
	}
	for (size_t i = 0; i < t->n_AttributeType; ++i)
	{
		int value = fight * t->AttributePro[i] / max_rate;
		ret->set_attr(t->AttributeType[i], value);

		if (t->AttributeType[i] < MAX_BUFF_FIGHT_ATTR)
		{
			ret->data->buff_fight_attr[t->AttributeType[i]] = value;
		}
	}

	for (size_t i = 0; i < t->n_Skill; ++i)
	{
		ret->m_skill.InsertSkill(t->Skill[i]);
	}
	ret->m_skill.adjust_ai_player_skill();

	ret->data->attrData[PLAYER_ATTR_HP] = ret->data->attrData[PLAYER_ATTR_MAXHP];
	
	// ret->data->move_path.pos[0].pos_x = 208;
	// ret->data->move_path.pos[0].pos_z = 42;
	ret->data->attrData[PLAYER_ATTR_MOVE_SPEED] = 5;	

	if (scene)
		scene->add_player_to_scene(ret);
	
		//登陆成功
	ret->data->status = ONLINE;
	
	player_manager_all_ai_players_id[player_id] = ret;
	return ret;
}

struct ai_player_data *player_manager::create_ai_data()
{
	struct ai_player_data *ret;
	if (player_manager_ai_data_free_list.empty())
	{
		ret = (struct ai_player_data *)malloc(sizeof(struct ai_player_data));
		if (!ret)
		{
			LOG_ERR("%s: malloc ai_data failed\n", __FUNCTION__);
			return NULL;
		}
	}
	else
	{
		ret = player_manager_ai_data_free_list.back();
		player_manager_ai_data_free_list.pop_back();
	}
	memset(ret, 0, sizeof(struct ai_player_data));
	player_manager_ai_data_used_list.insert(ret);
	return ret;
}

player_struct * player_manager::create_tmp_player(uint64_t player_id)
{
	player_struct *ret;	
	scene_struct *scene = NULL;
	LOG_DEBUG("%s %d: player_id[%lu]", __FUNCTION__, __LINE__, player_id);

	ret = get_player_by_id(player_id);
	ret = add_player(player_id);
	if (!ret) {
		LOG_ERR("%s %d: add player[%lu] fail", __FUNCTION__, __LINE__, player_id);		
		return NULL;
	}

	sprintf(ret->data->name, "player_%lu", player_id);
	ret->data->scene_id = 10000;
	ret->data->attrData[PLAYER_ATTR_LEVEL] = 1;
	ret->data->attrData[PLAYER_ATTR_JOB] = 1;
	ret->data->move_path.pos[0].pos_x = 208;
	ret->data->move_path.pos[0].pos_z = 42;
	ret->data->attrData[PLAYER_ATTR_MOVE_SPEED] = 5;	
//	ret->data->speed = 5;

	scene = scene_manager::get_scene(ret->data->scene_id);
	if (!scene) {
		LOG_ERR("%s %d: get scene[%u] fail", __FUNCTION__, __LINE__, ret->data->scene_id);
		return NULL;
	}
	scene->add_player_to_scene(ret);
	
		//登陆成功
	ret->data->status = ONLINE;
	return ret;
}

player_struct *player_manager::create_player(RAID_ENTER_REQUEST *req, uint64_t player_id)
{
	player_struct *ret = add_player(player_id);;	
	LOG_DEBUG("%s %d: player_id[%lu]", __FUNCTION__, __LINE__, player_id);
	if (!ret) {
		LOG_ERR("%s %d: add player[%lu] fail", __FUNCTION__, __LINE__, player_id);		
		return NULL;
	}
	LOG_DEBUG("[%s:%d] player_id:%lu, player:%p, data:%p", __FUNCTION__, __LINE__, ret->data->player_id, ret, ret->data);
	strncpy(ret->data->name, req->name, MAX_PLAYER_NAME_LEN);
	ret->data->name[MAX_PLAYER_NAME_LEN] = '\0';
	for (int i = 0; i < PLAYER_ATTR_MAX; ++i)
	{
		ret->data->attrData[i] = req->attrData[i];		
	}

	for (int i = 0; i < MAX_BUFF_FIGHT_ATTR; ++i)
	{
		ret->data->buff_fight_attr[i] = req->attrData[i];
	}

	for (int i = 0; i < MAX_MY_SKILL_NUM; ++i)
	{
		skill_struct *skill = skill_manager::copy_skill(&req->skill[i]);
		if (!skill)
			break;
		ret->m_skill.insert(skill);
	}
	ret->m_skill.m_index = req->skillindex;
	
	ret->data->status = ONLINE;
	ret->data->login_notify = true;
	return ret;
}

player_struct * player_manager::create_player(PROTO_ENTER_GAME_RESP *proto, uint64_t player_id)
{
	player_struct *ret;	
//	scene_struct *scene = NULL;
//	bool can_delete_when_fail = true;
	LOG_DEBUG("%s %d: player_id[%lu]", __FUNCTION__, __LINE__, player_id);

	ret = get_player_by_id(player_id);
	if (ret) {
		LOG_ERR("%s %d: player %lu status[%d] already in game, bug?", __FUNCTION__, __LINE__, player_id,ret->data->status);
		
		if (ret->data->status == ONLINE) {
			return ret;
		}

		//之前写回DB还没确认的玩家，如果此次登陆失败，不能删除，要等DB写回确认了才能删除
//		can_delete_when_fail = false;
		goto done;
	} else {
		ret = add_player(player_id);
	}
	if (!ret) {
		LOG_ERR("%s %d: add player[%lu] fail", __FUNCTION__, __LINE__, player_id);		
		return NULL;
	}
	LOG_DEBUG("[%s:%d] player_id:%lu, player:%p, data:%p", __FUNCTION__, __LINE__, ret->data->player_id, ret, ret->data);
	if (ret->unpack_dbinfo_to_playerinfo(proto->data, proto->data_size) != 0) {
		LOG_ERR("%s %d: unpack info[%lu] fail", __FUNCTION__, __LINE__, player_id);
		delete_player(ret);
		return NULL;
	}
	
	strncpy(ret->data->name, proto->name, MAX_PLAYER_NAME_LEN);
	ret->data->name[MAX_PLAYER_NAME_LEN] = '\0';
	ret->data->attrData[PLAYER_ATTR_LEVEL] = proto->lv<=0?1:proto->lv;
	ret->data->attrData[PLAYER_ATTR_JOB] = proto->job;
	ret->data->attrData[PLAYER_ATTR_REGION_ID] = -1;
	ret->data->chengjie.rest = proto->chengjie_cd;
	ret->data->guild_id = proto->guild_id;
	ret->data->guild_office = proto->guild_office;	
	ret->data->create_time = proto->create_time;
	if (ret->data->truck.truck_id != 0)
	{
		assert(ret->data->truck.scene_id > 0);
		scene_struct *pScene = scene_manager::get_scene(ret->data->truck.scene_id);
		if (pScene != NULL)
		{
			cash_truck_struct *truck = cash_truck_manager::create_cash_truck_at_pos(pScene, ret->data->truck.active_id, *ret, ret->data->truck.pos.pos_x, ret->data->truck.pos.pos_z, ret->data->truck.hp);
			ret->data->truck.truck_id = truck->get_uuid();
		}
	}

done:
/*	
	scene_struct *scene = scene_manager::get_scene(ret->data->scene_id);
	if (!scene) {
		LOG_ERR("%s %d: get scene[%lu] fail", __FUNCTION__, __LINE__, ret->data->scene_id);
		if (can_delete_when_fail)
			delete_player(ret);
		return NULL;
	}
	scene->add_player_to_scene(ret);
*/
	
//	ret->data->raid = NULL;
//	ret->data->raid_uuid = 0;
	
		//登陆成功
	ret->data->status = ONLINE;
//	ret->srtt = 0;
	return ret;
}
