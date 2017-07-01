#include "game_event.h"
#include "player_manager.h"
#include "scene_manager.h"
#include "scene.h"
#include "skill_manager.h"
#include "mem_pool.h"
#include "uuid.h"
#include "game_config.h"
#include "lua_config.h"
#include "conn_node_gamesrv.h"
#include <stdio.h>
#include <errno.h>

player_manager::player_manager()
{
}

player_manager::~player_manager()
{
}


/////////////////////  以下是静态成员
ai_player_handle player_manager::m_ai_player_handle;
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
	}
	return init_comm_pool(0, sizeof(player_data), num, key, &player_manager_player_data_pool);
}
/*
int player_manager::resume_player_struct(int num, unsigned long key)
{
	scene_struct *scene;
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
		LOG_DEBUG("%s %d: status[%d], player_id[%lu], name[%s] posx[%f] posy[%f]\n",
			__FUNCTION__, __LINE__, data->status, data->player_id, data->name, data->attrData[PLAYER_ATTR_POSX], data->attrData[PLAYER_ATTR_POSY]);
		player = player_manager_player_free_list.back();
		if (!player) {
			LOG_ERR("%s %d: get free player failed", __FUNCTION__, __LINE__);
			return (-1);
		}
		player_manager_player_free_list.pop_back();	
		player->data = data;
		data->scene = NULL;
		data->raid = NULL;		

		if (resume_player_bag_data(player) != 0) {
			LOG_ERR("%s %d: resume player bag data failed", __FUNCTION__, __LINE__);
			return (-10);			
		}
		
		add_player(player);
		if (data->status == ONLINE) {
			scene = scene_manager::get_scene(player->get_scene_id());
			if (!scene) {
				LOG_ERR("%s %d: get scene[%d] fail", __FUNCTION__, __LINE__, player->get_scene_id());
				return (-20);
			}
			scene->add_player_to_scene(player, false);

			raid_struct *raid = raid_manager::get_raid_by_id(data->raid_uuid);
			if (raid) {
//				raid->add_player_to_scene(player, false);
				raid->resume_player_to_scene(player);
				data->raid = raid;

				if (!raid->data->is_start) {
					raid_manager::insert_to_wait(raid->data->raid_id, raid);
				}
			}


			player->check_power();	/// 每次登陆时重计算一下体力
			player->map_task();		/// 映射已完成的任务列表
//			player->create_index_task();
			player->item_index_create();
			player->temp_index_create();
			player->map_dragon();
			player->load_raid_to_cache();
			player->load_achievement_to_cache();
			player->load_daily_quest_to_cache();
			player->check_skin_ts();
		}
	}
	return (ret);
}

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
	data = (player_data *)comm_pool_alloc(&player_manager_player_data_pool);
	if (!data)
		goto fail;
	memset(data, 0, sizeof(player_data));
	ret->data = data;
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

	p->clear_all_buffs();
	p->interrupt();
	p->clear_temp_data();
	p->clear_all_partners();
	p->clear_all_escort();
	
	LOG_DEBUG("[%s:%d] player_id[%lu], player:%p, data:%p", __FUNCTION__, __LINE__, p->data->player_id, p, p->data);

	if (get_entity_type(p->data->player_id) == ENTITY_TYPE_AI_PLAYER)
		player_manager_all_ai_players_id.erase(p->data->player_id);

	if (p->data) {
		remove_player(p);
		comm_pool_free(&player_manager_player_data_pool, p->data);		
		p->data = NULL;
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
		// TODO: ai player ai
	for (std::map<uint64_t, player_struct *>::iterator iter = player_manager_all_ai_players_id.begin(); iter != player_manager_all_ai_players_id.end(); ++iter)
	{
		player_struct *player = iter->second;
		if (!player->scene)
			continue;
		if (m_ai_player_handle)
			m_ai_player_handle(player);
	}
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

player_struct * player_manager::create_ai_player(player_struct *player, scene_struct *scene, int name_index)
{
	player_struct *ret;
	uint64_t player_id = alloc_ai_player_uuid();
	
	LOG_DEBUG("%s %d: player_id[%lu] ai[%lu]", __FUNCTION__, __LINE__, player->get_uuid(), player_id);


	ret = get_player_by_id(player_id);
	ret = add_player(player_id);
	if (!ret) {
		LOG_ERR("%s %d: add player[%lu] fail", __FUNCTION__, __LINE__, player_id);		
		return NULL;
	}

	strcpy(ret->data->name, get_rand_player_name(name_index));

	if (scene)
		ret->data->scene_id = scene->m_id;

	int index = random() % robot_config.size();	

	int fight = player->get_attr(PLAYER_ATTR_FIGHTING_CAPACITY);
//	fight += random() % (sg_pvp_raid_fighting_capacity_range[1] - sg_pvp_raid_fighting_capacity_range[0])
//		+ sg_pvp_raid_fighting_capacity_range[0];

	fight += random() % (robot_config[index]->FightPro[1] - robot_config[index]->FightPro[0])
		+ robot_config[index]->FightPro[0];
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
	ret->data->attrData[PLAYER_ATTR_JOB] = robot_config[index]->ID % 16;//index + 1;
	ret->data->attrData[PLAYER_ATTR_HEAD] = robot_config[index]->InitialHead;

		//时装，武器，头饰
	ret->data->attrData[PLAYER_ATTR_WEAPON] = robot_config[index]->WeaponId[random() % robot_config[index]->n_WeaponId];
	ret->data->attrData[PLAYER_ATTR_CLOTHES] = robot_config[index]->ResId[random() % robot_config[index]->n_ResId];
	ret->data->attrData[PLAYER_ATTR_HAT] = robot_config[index]->HairResId[random() % robot_config[index]->n_HairResId];

		//ai
	ret->data->active_attack_range = robot_config[index]->ActiveAttackRange;
	ret->data->chase_range = robot_config[index]->ChaseRange;

	int max_rate = 0;
	for (size_t i = 0; i < robot_config[index]->n_AttributeType; ++i)
	{
		max_rate += robot_config[index]->AttributePro[i];
	}
	for (size_t i = 0; i < robot_config[index]->n_AttributeType; ++i)
	{
		int value = fight * robot_config[index]->AttributePro[i] / max_rate;
		ret->set_attr(robot_config[index]->AttributeType[i], value);
	}

	for (size_t i = 0; i < robot_config[index]->n_Skill; ++i)
	{
		ret->m_skill.InsertSkill(robot_config[index]->Skill[i]);
	}

	ret->data->attrData[PLAYER_ATTR_HP] = ret->data->attrData[PLAYER_ATTR_MAXHP];
	
	// ret->data->move_path.pos[0].pos_x = 208;
	// ret->data->move_path.pos[0].pos_z = 42;
	ret->data->attrData[PLAYER_ATTR_MOVE_SPEED] = 5;	

	if (scene)
		scene->add_player_to_scene(ret);
	
		//登陆成功
	ret->data->status = ONLINE;

		//停止AI
	ret->data->stop_ai = true;

	player_manager_all_ai_players_id[player_id] = ret;
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
