#include "game_event.h"
#include "monster_manager.h"
#include "player_manager.h"
#include "sight_space_manager.h"
#include "mem_pool.h"
#include "conn_node_gamesrv.h"
#include "uuid.h"
#include "msgid.h"
#include "game_config.h"
#include "scene_manager.h"
#include <stdio.h>
#include <errno.h>
#include "../proto/rank_db.pb-c.h"
#include "conn_node.h"
#include "chat.pb-c.h"
#include "team.h"
#include "../proto/rank.pb-c.h"


monster_manager::monster_manager()
{
}

monster_manager::~monster_manager()
{
}


/////////////////////  以下是静态成员
// struct minheap monster_manager::monster_manager_m_minheap;
// std::map<uint64_t, monster_struct *> monster_manager::monster_manager_all_monsters_id;
// std::list<monster_struct *> monster_manager::monster_manager_monster_free_list;
// std::set<monster_struct *> monster_manager::monster_manager_monster_used_list;
// comm_pool monster_manager::monster_manager_monster_data_pool;

// struct minheap monster_manager::monster_manager_m_boss_minheap;
// std::map<uint64_t, boss_struct *> monster_manager::monster_manager_all_boss_id;
// std::list<boss_struct *> monster_manager::monster_manager_boss_free_list;
// std::set<boss_struct *> monster_manager::monster_manager_boss_used_list;
// comm_pool monster_manager::monster_manager_boss_data_pool;

int monster_manager::add_monster(monster_struct *p)
{
	monster_manager_all_monsters_id[p->data->player_id] = p;
	//世界boss表，已约定世界boss的怪物id是所有场景唯一的，所以以怪物id为索引
	if(p->config->Type == 5 && monster_to_world_boss_config.find(p->data->monster_id) !=  monster_to_world_boss_config.end())
	{
		world_boss_all_monsters_id[p->data->monster_id] = p;
	}
	return (0);
}

int monster_manager::remove_monster(monster_struct *p)
{
	monster_manager_all_monsters_id.erase(p->data->player_id);
	//世界boss表
	if(p->config->Type == 5 && monster_to_world_boss_config.find(p->data->monster_id) !=  monster_to_world_boss_config.end())
	{
		world_boss_all_monsters_id.erase(p->data->monster_id);
	}
	return (0);
}

monster_struct * monster_manager::get_monster_by_id(uint64_t id)
{
	std::map<uint64_t, monster_struct *>::iterator it = monster_manager_all_monsters_id.find(id);
	if (it != monster_manager_all_monsters_id.end())
	{
		if (it->second->mark_delete)
		{
			LOG_INFO("%s: monster[%lu] already mark delete", __FUNCTION__, id);
			return NULL;
		}
		return it->second;
	}

//	return get_boss_by_id(id);
	
	return NULL;
}

int monster_manager::reset_all_monster_ai()
{
	{
		std::map<uint64_t, monster_struct *>::iterator it = monster_manager_all_monsters_id.begin();
		for (; it != monster_manager_all_monsters_id.end(); ++it)
		{
			it->second->set_ai_interface(it->second->ai_type);
		}
	}

	// {
	// 	std::map<uint64_t, boss_struct *>::iterator it = monster_manager_all_boss_id.begin();
	// 	for (; it != monster_manager_all_boss_id.end(); ++it)
	// 	{
	// 		it->second->set_ai_interface(it->second->ai_type);
	// 	}
	// }
	return (0);
}

//////////////////////////////////////////////
static bool minheap_cmp_monster_timeout(void *a, void *b)
{
	monster_struct *aa = (monster_struct *)a;
	monster_struct *bb = (monster_struct *)b;
	if (aa->data->ontick_time < bb->data->ontick_time)
		return true;
	return false;
}

static int minheap_get_monster_timeout_index(void *a)
{
	monster_struct *aa = (monster_struct *)a;
	return aa->data->heap_index;
}

static void minheap_set_monster_timeout_index(int index, void *a)
{
	monster_struct *aa = (monster_struct *)a;
	aa->data->heap_index = index;
}

void monster_manager::monster_ontick_settimer(monster_struct *p)
{
	assert(p->mark_delete == false);
	switch (p->config->HateType)
	{
		case MONSTER_HATETYPE_DEFINE_BOSS:
		case MONSTER_HATETYPE_DEFINE_AIBOSS:
			push_heap(&monster_manager_m_boss_minheap, p);			
			break;
		default:
			push_heap(&monster_manager_m_minheap, p);
			break;
	}
}

void monster_manager::monster_ontick_reset_timer(monster_struct *p)
{
	assert(p->mark_delete == false);
	switch (p->config->HateType)
	{
		case MONSTER_HATETYPE_DEFINE_BOSS:
		case MONSTER_HATETYPE_DEFINE_AIBOSS:			
			adjust_heap_node(&monster_manager_m_boss_minheap, p);			
			break;
		default:
			adjust_heap_node(&monster_manager_m_minheap, p);
			break;
	}
}

monster_struct *monster_manager::get_ontick_monster(uint64_t now)
{
	if (monster_manager_m_minheap.cur_size == 0)
		return NULL;

	if (((monster_struct *)get_heap_first(&monster_manager_m_minheap))->data->ontick_time > now)
		return NULL;
	return (monster_struct *)pop_heap(&monster_manager_m_minheap);
}

void monster_manager::monster_ontick_delete(monster_struct *p)
{
	switch (p->config->HateType)
	{
		case MONSTER_HATETYPE_DEFINE_BOSS:
		case MONSTER_HATETYPE_DEFINE_AIBOSS:			
			if (is_node_in_heap(&monster_manager_m_boss_minheap, p))
				erase_heap_node(&monster_manager_m_boss_minheap, p);
			break;
		default:
			if (is_node_in_heap(&monster_manager_m_minheap, p))			
				erase_heap_node(&monster_manager_m_minheap, p);
			break;
	}
}

//////////////////////////////////////////

int monster_manager::init_monster_struct(int num, unsigned long key)
{
		//BOSS怪物数量写死是普通怪物的20%
	init_heap(&monster_manager_m_minheap, num, minheap_cmp_monster_timeout, minheap_get_monster_timeout_index, minheap_set_monster_timeout_index);
 	init_heap(&monster_manager_m_boss_minheap, num / 5, minheap_cmp_monster_timeout, minheap_get_monster_timeout_index, minheap_set_monster_timeout_index);

	monster_struct *monster;
	for (int i = 0; i < num; ++i) {
		monster = new monster_struct();
		monster_manager_monster_free_list.push_back(monster);
#ifdef __RAID_SRV__
		monster->data = (monster_data *)malloc(sizeof(monster_data));
#endif		
	}
#ifdef __RAID_SRV__
	return (0);
#else
	LOG_DEBUG("%s: init mem[%lu][%lu]", __FUNCTION__, sizeof(monster_struct) * num, sizeof(monster_data) * num);			
	return init_comm_pool(0, sizeof(monster_data), num, key, &monster_manager_monster_data_pool);
#endif
}
/*
int monster_manager::resume_monster_struct(int num, unsigned long key)
{
	scene_struct *scene;
	monster_struct *monster;
	for (int i = 0; i < num; ++i) {
		monster = new monster_struct();
		monster_manager_monster_free_list.push_back(monster);
	}

	int ret = init_comm_pool(1, sizeof(monster_data), num, key, &monster_manager_monster_data_pool);
	int index = 0;
	for (;;) {
		struct monster_data *data = (struct monster_data *)get_next_inuse_comm_pool_entry(&monster_manager_monster_data_pool, &index);
		if (!data)
			break;
		LOG_DEBUG("%s %d: status[%d], monster_id[%lu], name[%s] posx[%f] posy[%f]\n",
			__FUNCTION__, __LINE__, data->status, data->monster_id, data->name, data->attrData[MONSTER_ATTR_POSX], data->attrData[MONSTER_ATTR_POSY]);
		monster = monster_manager_monster_free_list.back();
		if (!monster) {
			LOG_ERR("%s %d: get free monster failed", __FUNCTION__, __LINE__);
			return (-1);
		}
		monster_manager_monster_free_list.pop_back();
		monster->data = data;
		data->scene = NULL;
		data->raid = NULL;

		if (resume_monster_bag_data(monster) != 0) {
			LOG_ERR("%s %d: resume monster bag data failed", __FUNCTION__, __LINE__);
			return (-10);
		}

		add_monster(monster);
		if (data->status == ONLINE) {
			scene = scene_manager::get_scene(monster->get_scene_id());
			if (!scene) {
				LOG_ERR("%s %d: get scene[%d] fail", __FUNCTION__, __LINE__, monster->get_scene_id());
				return (-20);
			}
			scene->add_monster_to_scene(monster, false);

			raid_struct *raid = raid_manager::get_raid_by_id(data->raid_uuid);
			if (raid) {
//				raid->add_monster_to_scene(monster, false);
				raid->resume_monster_to_scene(monster);
				data->raid = raid;

				if (!raid->data->is_start) {
					raid_manager::insert_to_wait(raid->data->raid_id, raid);
				}
			}


			monster->check_power();	/// 每次登陆时重计算一下体力
			monster->map_task();		/// 映射已完成的任务列表
//			monster->create_index_task();
			monster->item_index_create();
			monster->temp_index_create();
			monster->map_dragon();
			monster->load_raid_to_cache();
			monster->load_achievement_to_cache();
			monster->load_daily_quest_to_cache();
			monster->check_skin_ts();
		}
	}
	return (ret);
}

int monster_manager::resume_monster_bag_data(monster_struct *monster)
{
	for (int i = 0; i < monster->data->active_equip_bag_grids; ++i) {
		if (monster->data->equip_grids[i].id == 0 || monster->data->equip_grids[i].uuid == 0)
			continue;
		equip_struct *equip = equip_manager::get_equip_by_id(monster->data->equip_grids[i].uuid);
		if (!equip) {
			LOG_ERR("%s %d: resume equip %lu failed", __FUNCTION__, __LINE__, monster->data->equip_grids[i].uuid);
			return (-1);
		}
		monster->data->equip_grids[i].equip = equip;
	}
	for (int i = 0; i < EQUIPPOSDEFINE_END; ++i) {
		if (monster->data->used_equip_grids[i].id == 0 || monster->data->used_equip_grids[i].uuid == 0)
			continue;
		equip_struct *equip = equip_manager::get_equip_by_id(monster->data->used_equip_grids[i].uuid);
		if (!equip) {
			LOG_ERR("%s %d: resume equip %lu failed", __FUNCTION__, __LINE__, monster->data->used_equip_grids[i].uuid);
			return (-1);
		}
		monster->data->used_equip_grids[i].equip = equip;
	}
	return (0);
}
*/
monster_struct *monster_manager::alloc_monster()
{
	monster_struct *ret = NULL;
	monster_data *data = NULL;
	if (monster_manager_monster_free_list.empty())
		return NULL;
	ret = monster_manager_monster_free_list.back();
	monster_manager_monster_free_list.pop_back();
#ifdef __RAID_SRV__
	if (!ret)
		goto fail;
	memset(ret->data, 0, sizeof(monster_data));	
#else		
	data = (monster_data *)comm_pool_alloc(&monster_manager_monster_data_pool);
	if (!data)
		goto fail;
	memset(data, 0, sizeof(monster_data));
	ret->data = data;
#endif	
	monster_manager_monster_used_list.insert(ret);
	ret->init_monster();
	return ret;
fail:
	if (ret) {
		monster_manager_monster_used_list.erase(ret);
		monster_manager_monster_free_list.push_back(ret);
	}
	if (data) {
		comm_pool_free(&monster_manager_monster_data_pool, data);
	}
	return NULL;
}

void monster_manager::delete_monster(monster_struct *p)
{
	LOG_DEBUG("%s: %p", __FUNCTION__, p);
		//位面的怪物随着位面一起删除
	if (p->sight_space)
		return;
	if (p->mark_delete)
		return;

	if (p->data && p->data->owner)
	{
		player_struct *owner = player_manager::get_player_by_id(p->data->owner);
		if (owner)
		{
			owner->del_pet(p);
		}
	}
	p->clear_all_buffs();
	p->clear_monster_timer();
	p->mark_delete = true;
	monster_manager_delete_list.push_back(p);
	
	// switch ((int)p->get_unit_type())
	// {
	// 	case UNIT_TYPE_MONSTER:
	// 		delete_monster_impl(p);
	// 		break;
	// 	case UNIT_TYPE_BOSS:
	// 		delete_boss_impl((boss_struct *)p);
	// 		break;
	// }
}

void monster_manager::delete_monster_impl(monster_struct *p)
{
	LOG_DEBUG("%s %d: monster[%p] data[%p]", __FUNCTION__, __LINE__, p, p->data);

	// if (p->data && p->data->owner)
	// {
	// 	player_struct *owner = player_manager::get_player_by_id(p->data->owner);
	// 	if (owner)
	// 	{
	// 		owner->del_pet(p);
	// 	}
	// }

	monster_manager_monster_used_list.erase(p);
	monster_manager_monster_free_list.push_back(p);

	// p->clear_all_buffs();
	// if (is_node_in_heap(&monster_manager_m_minheap, p))
	// 	monster_ontick_delete(p);

	if (p->data) {
		LOG_INFO("[%s:%d] monster_id[%u], uuid[%lu]", __FUNCTION__, __LINE__, p->data->monster_id, p->data->player_id);
		remove_monster(p);
#ifdef __RAID_SRV__
#else						
		comm_pool_free(&monster_manager_monster_data_pool, p->data);
		p->data = NULL;
#endif		
	} else {
		LOG_ERR("[%s:%d] monster[%p]", __FUNCTION__, __LINE__, p);
	}

}

void monster_manager::on_tick_1()
{
}

void monster_manager::on_tick_5()
{
	uint64_t now = time_helper::get_cached_time();
	monster_struct *boss = get_ontick_boss(now);
	while (boss != NULL)
	{
		boss->data->ontick_time = now + boss->ai_config->Response;
		if (!boss->data->stop_ai)
			boss->on_tick();
		if (boss->data && !boss->mark_delete)
			boss_ontick_settimer(boss);
		boss = get_ontick_boss(now);
	}
}

void monster_manager::on_tick_10()
{
	uint64_t now = time_helper::get_cached_time();
	monster_struct *monster = get_ontick_monster(now);
	while (monster != NULL)
	{
		monster->data->ontick_time = now + monster->ai_config->Response;
		if (!monster->data->stop_ai)
			monster->on_tick();
		if (monster->m_liveTime != 0 && now > monster->m_liveTime)
		{
			if (monster->config->Type == 4)  //寻宝boss
			{
				player_struct *player = player_manager::get_player_by_id(monster->data->owner);
				if (player != NULL)
				{
					//系统提示
					player->send_system_notice(190500285, NULL);
				}
			}
			if (monster->sight_space != NULL)
			{
				sight_space_manager::mark_sight_space_delete(monster->sight_space);  //位面副本 等出位面再删
			}
			else
			{
				monster->scene->delete_monster_from_scene(monster, true);
				delete_monster(monster);
			}
			monster = get_ontick_monster(now);
			continue;
		}
		if (monster->data && !monster->mark_delete)
			monster_ontick_settimer(monster);
		monster = get_ontick_monster(now);
	}
}

void monster_manager::on_tick_30()
{
}
void monster_manager::on_tick_50()
{
	for (std::list<monster_struct *>::iterator ite = monster_manager_delete_list.begin();
		 ite != monster_manager_delete_list.end(); ++ite)
	{
		assert((*ite)->mark_delete == true);
		delete_monster_impl(*ite);
		
		// switch ((int)(*ite)->get_unit_type())
		// {
		// 	case UNIT_TYPE_MONSTER:
		// 		delete_monster_impl(*ite);
		// 		break;
		// 	case UNIT_TYPE_BOSS:
		// 		delete_boss_impl((boss_struct *)(*ite));
		// 		break;
		// }
	}
	monster_manager_delete_list.clear();
}

void monster_manager::on_tick_100()
{
//	for (std::set<monster_struct *>::iterator iter = monster_manager_monster_used_list.begin(); iter != monster_manager_monster_used_list.end(); ++iter)
//	{
//		(*iter)->on_tick();
//	}
}

void monster_manager::on_tick_500()
{
}

void monster_manager::on_tick_1000()
{

}

unsigned int monster_manager::get_monster_pool_max_num()
{
	return monster_manager_monster_data_pool.num;
}
// unsigned int monster_manager::get_boss_pool_max_num()
// {
// 	return monster_manager_boss_data_pool.num;
// }

monster_struct *monster_manager::add_monster(uint64_t monster_id, uint64_t lv, unit_struct *owner)
{
	std::map<uint64_t, struct MonsterTable *>::iterator ite;
	ite = monster_config.find(monster_id);
	if (ite == monster_config.end())
		return NULL;

	//刷世界boss
	if(ite->second->Type == 5)
	{
		uint32_t cd = 0;
		if(!check_active_open(WORD_BOSS_ACTIVE_ID, cd))
		{
			return NULL;
		}

		std::map<uint64_t, struct WorldBossTable *>::iterator p = monster_to_world_boss_config.find(monster_id);
		if(p == monster_to_world_boss_config.end())
		{
			return NULL;
		}
	}

	LOG_DEBUG("%s: monster[%lu] ai[%lu] lv[%lu]",  __FUNCTION__, monster_id, ite->second->BaseID, lv);
	
	monster_struct *ret;
	ret = alloc_monster();
	if (!ret)
		return NULL;
	ret->data->player_id = alloc_monster_uuid();

	// switch (ite->second->HateType)
	// {
	// 	case MONSTER_TYPE_DEFINE_BOSS:
	// 	{
	// 		boss_struct *boss = alloc_boss();
	// 		if (!boss)
	// 			return NULL;
	// 		boss->data->player_id = alloc_monster_uuid();
	// 		add_boss(boss);
	// 		ret = boss;
	// 	}
	// 	break;
	// 	default:
	// 	{
	// 		ret = alloc_monster();
	// 		if (!ret)
	// 			return NULL;
	// 		ret->data->player_id = alloc_monster_uuid();
	// 		add_monster(ret);
	// 	}
	// 	break;
	// }

	ret->config = ite->second;
	ret->ai_config = base_ai_config[ret->config->BaseID];
	assert(ret->ai_config);
	ret->ai_type = ret->ai_config->AIType;
	ret->ai_state = AI_PATROL_STATE;
	ret->set_ai_interface(ret->ai_type);
	ret->data->monster_id = monster_id;
	ret->data->birth_time = time_helper::get_cached_time()/1000;
	ret->set_attr(PLAYER_ATTR_LEVEL, lv);
		// 阵营模式, PK模式
	ret->set_attr(PLAYER_ATTR_PK_TYPE, ret->config->PkType);
	ret->set_attr(PLAYER_ATTR_ZHENYING, ret->config->Camp);
	
	// switch (ret->config->Camp)
	// {
	// 	case 1:
	// 		ret->set_attr(PLAYER_ATTR_PK_TYPE, 4);
	// 		ret->set_attr(PLAYER_ATTR_ZHENYING, 1);
	// 		break;
	// 	case 2:
	// 		ret->set_attr(PLAYER_ATTR_PK_TYPE, 4);
	// 		ret->set_attr(PLAYER_ATTR_ZHENYING, 2);
	// 		break;
	// 	case 3:
	// 		ret->set_attr(PLAYER_ATTR_PK_TYPE, 5);
	// 		break;
	// 	default:
	// 	//默认pk模式是怪物
	// 		ret->set_attr(PLAYER_ATTR_PK_TYPE, 3);
	// 		break;
	// }
	add_monster(ret);
	if (owner && owner->get_unit_type() == UNIT_TYPE_PLAYER)
	{
		((player_struct *)owner)->add_pet(ret);
	}
	ret->calculate_attribute();
	if (ret->config->LifeTime == 0)
	{
		ret->m_liveTime = 0;
	}
	else
	{
		ret->m_liveTime = time_helper::get_cached_time() + ret->config->LifeTime * 1000;
	}

	if (ret->ai && ret->ai->on_monster_ai_init)
		ret->ai->on_monster_ai_init(ret, owner);
	
	LOG_DEBUG("%s: add monster[%p]data[%p] %u %lu, type %lu", __FUNCTION__, ret, ret->data, ret->data->monster_id, ret->data->player_id, ret->config->HateType);
	return ret;
}

monster_struct *monster_manager::create_call_monster(player_struct *player, SkillTable *skill_config)
{
	SkillLvTable *lv_config = get_config_by_id(skill_config->SkillLv, &skill_lv_config);
	if (!lv_config)
		return NULL;
	if (lv_config->MonsterID == 0 || lv_config->MonsterLv == 0)
		return NULL;

	if (player->sight_space)
	{
		return monster_manager::create_sight_space_monster(player->sight_space, player->scene, lv_config->MonsterID, lv_config->MonsterLv, player->get_pos()->pos_x, player->get_pos()->pos_z, player);		
	}
	
	monster_struct *monster = add_monster(lv_config->MonsterID, lv_config->MonsterLv, player);
	if (!monster)
		return NULL;
	monster->create_config = NULL;
	monster->born_pos.pos_x = player->get_pos()->pos_x;
	monster->born_pos.pos_z = player->get_pos()->pos_z;			
	monster->data->create_config_index = -1;
	monster->set_pos(player->get_pos()->pos_x, player->get_pos()->pos_z);

	if (player->scene->add_monster_to_scene(monster, lv_config->MonsterEff) != 0)
	{
		LOG_ERR("%s: uuid[%lu] monster[%lu] scene[%u]", __FUNCTION__, monster->data->player_id, lv_config->MonsterID, player->scene->m_id);
		delete_monster(monster);
	}

	return monster;
}

monster_struct *monster_manager::create_sight_space_monster(sight_space_struct *sight_space, scene_struct *scene, uint32_t monster_id, uint32_t level, double pos_x, double pos_z, unit_struct *owner)
{
	for (int i = 0; i < MAX_MONSTER_IN_SIGHT_SPACE; ++i)
	{
		if (sight_space->monsters[i] != NULL)
			continue;

		monster_struct *monster = add_monster(monster_id, level, owner);
		if (!monster)
			return NULL;

		monster->create_config = NULL;
		monster->born_pos.pos_x = pos_x;
		monster->born_pos.pos_z = pos_z;		
		monster->data->create_config_index = -1;
		monster->set_pos(pos_x, pos_z);
		monster->sight_space = sight_space;
		sight_space->monsters[i] = monster;
//		sight_space->data->monster_uuid[i] = monster->get_uuid();

		if (monster->ai && monster->ai->on_alive)
			monster->ai->on_alive(monster);
		sight_space->broadcast_monster_create(monster);
		monster->scene = scene;
		if (sight_space->players[0] && sight_space->players[0]->data->playing_drama)
		{
			monster->data->stop_ai = true;
		}
		if (owner != NULL)
		{
			++sight_space->data->n_monster_call;
		}
		return monster;
	}
	return NULL;
}

// monster_struct *monster_manager::create_sight_only_monster(player_struct *player, uint32_t monster_id, uint32_t level, double pos_x, double pos_z)
// {
// 	assert(player->is_avaliable());

// 	if (player->data->cur_sight_monster >= MAX_MONSTER_IN_PLAYER_SIGHT)
// 		return NULL;

// 	monster_struct *monster = add_monster(monster_id, level);
// 	if (!monster)
// 		return NULL;
// //	assert(monster->ai_type == 5);
// 	monster->ai_type = 5;
// 	monster->set_ai_interface(5);

// 	monster->set_pos(pos_x, pos_z);
// 	monster->data->scene_id = player->scene->m_id;
// 	monster->scene = player->scene;
// 	player->add_monster_to_sight_both(monster);

// 	if (monster->ai && monster->ai->on_alive)
// 		monster->ai->on_alive(monster);

// 	SightChangedNotify notify;
// 	sight_changed_notify__init(&notify);
// 	SightMonsterInfo monster_info[1];
// 	SightMonsterInfo *monster_info_point[1];
// 	monster_info_point[0] = &monster_info[0];
// 	notify.n_add_monster = 1;
// 	notify.add_monster = monster_info_point;
// 	monster->pack_sight_monster_info(monster_info_point[0]);

// 	EXTERN_DATA extern_data;
// 	extern_data.player_id = player->data->player_id;
// 	fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, MSG_ID_SIGHT_CHANGED_NOTIFY, sight_changed_notify__pack, notify);
// 	return monster;
// }

monster_struct *monster_manager::create_monster_at_pos(scene_struct *scene, uint64_t id, uint32_t lv, int32_t pos_x, int32_t pos_z, uint32_t effectid, unit_struct *owner, float direct)
{
	if(lv <= 0)
		return NULL;
	monster_struct *monster = add_monster(id, lv, owner);
	if (!monster)
		return NULL;
	monster->data->create_config_index = 0;
	monster->create_config = NULL;
	monster->born_pos.pos_x = pos_x;
	monster->born_pos.pos_z = pos_z;
	monster->data->born_direct = direct;
	monster->set_pos(pos_x, pos_z);

	if (scene != NULL)
	{
		if (scene->add_monster_to_scene(monster, effectid) != 0)
		{
			LOG_ERR("%s: uuid[%lu] monster[%lu] scene[%u]", __FUNCTION__, monster->data->player_id, id, scene->m_id);
		}
	}
	
	return monster;
}

int monster_manager::create_monster_by_id(scene_struct *scene, uint32_t id, uint32_t num, uint64_t  monster_level)
{
	if (!scene->create_monster_config)
	{
		return (-1);
	}
	int len = scene->create_monster_config->size();
	for (int i = 0; i < len && num > 0; ++i)
	{
		struct SceneCreateMonsterTable *create_config = (*scene->create_monster_config)[i];
		if (!create_config)
			continue;
		if (create_config->ID != id)
			continue;
		create_monster_by_config(scene, i, monster_level);
		--num;
	}

	return (0);
}

monster_struct *monster_manager::create_monster_by_config(scene_struct *scene, int index, uint64_t monster_level)
{
	struct SceneCreateMonsterTable *create_config = (*scene->create_monster_config)[index];
	if (!create_config)
		return NULL;

	monster_level = (monster_level == 0 ? create_config->Level : monster_level);
	monster_struct *monster = add_monster(create_config->ID, monster_level);
	if (!monster)
		return NULL;
	monster->data->create_config_index = index;
	monster->data->born_direct = create_config->Yaw;
	monster->create_config = create_config;
	monster->born_pos.pos_x = create_config->PointPosX;
	monster->born_pos.pos_z = create_config->PointPosZ;	
	switch (monster->ai_type)
	{
		case AI_TYPE_CIRCLE:
		case AI_TYPE_ESCORT:
			if (create_config->n_TargetInfoList == 0)
			{
				LOG_ERR("%s: scene[%u] create monster[%lu] wrong", __FUNCTION__, scene->m_id, create_config->ID);
				assert(0);
			}
			monster->set_pos(create_config->TargetInfoList[0]->TargetPos->TargetPosX,
				create_config->TargetInfoList[0]->TargetPos->TargetPosZ);
			break;
//		case AI_TYPE_NORMAL:
		default:
			monster->set_pos(monster->get_born_pos_x(), monster->get_born_pos_z());
			break;
	}
	uint32_t effectid = 0;
	if (scene->add_monster_to_scene(monster, effectid) != 0)
	{
		LOG_ERR("%s: uuid[%lu] monster[%lu] scene[%u]", __FUNCTION__, monster->data->player_id, create_config->ID, scene->m_id);
	}
	return monster;
}

//////////////////////////////boss

// int monster_manager::add_boss(boss_struct *p)
// {
// 	monster_manager_all_boss_id[p->data->player_id] = p;
// 	return (0);
// }

// int monster_manager::remove_boss(boss_struct *p)
// {
// 	monster_manager_all_boss_id.erase(p->data->player_id);
// 	return (0);
// }

// boss_struct * monster_manager::get_boss_by_id(uint64_t id)
// {
// 	std::map<uint64_t, boss_struct *>::iterator it = monster_manager_all_boss_id.find(id);
// 	if (it != monster_manager_all_boss_id.end())
// 	{
// 		if (it->second->mark_delete)
// 		{
// 			LOG_INFO("%s: boss[%lu] already mark delete", __FUNCTION__, id);
// 			return NULL;
// 		}
// 		return it->second;
// 	}
// 	return NULL;
// }

//////////////////////////////////////////////
void monster_manager::boss_ontick_settimer(monster_struct *p)
{
	push_heap(&monster_manager_m_boss_minheap, p);
}

// void monster_manager::boss_ontick_reset_timer(monster_struct *p)
// {
// 	adjust_heap_node(&monster_manager_m_boss_minheap, p);
// }

monster_struct *monster_manager::get_ontick_boss(uint64_t now)
{
	if (monster_manager_m_boss_minheap.cur_size == 0)
		return NULL;

	if (((monster_struct *)get_heap_first(&monster_manager_m_boss_minheap))->data->ontick_time > now)
		return NULL;
	return (monster_struct *)pop_heap(&monster_manager_m_boss_minheap);
}

// void monster_manager::boss_ontick_delete(monster_struct *p)
// {
// 	erase_heap_node(&monster_manager_m_boss_minheap, p);
// }

//////////////////////////////////////////

int monster_manager::reinit_monster_min_heap()
{
	monster_manager_m_minheap.cmp = minheap_cmp_monster_timeout;
	monster_manager_m_minheap.get = minheap_get_monster_timeout_index;
	monster_manager_m_minheap.set = minheap_set_monster_timeout_index;
	return (0);
}

int monster_manager::reinit_boss_min_heap()
{
	monster_manager_m_boss_minheap.cmp = minheap_cmp_monster_timeout;
	monster_manager_m_boss_minheap.get = minheap_get_monster_timeout_index;
	monster_manager_m_boss_minheap.set = minheap_set_monster_timeout_index;
	return (0);
}

// int monster_manager::init_boss_struct(int num, unsigned long key)
// {
// 	init_heap(&monster_manager_m_boss_minheap, num, minheap_cmp_monster_timeout, minheap_get_monster_timeout_index, minheap_set_monster_timeout_index);

// 	boss_struct *monster;
// 	for (int i = 0; i < num; ++i) {
// 		monster = new boss_struct();
// 		monster_manager_boss_free_list.push_back(monster);
// 	}
// 	return init_comm_pool(0, sizeof(monster_data), num, key, &monster_manager_boss_data_pool);
// }

// boss_struct *monster_manager::alloc_boss()
// {
// 	boss_struct *ret = NULL;
// 	monster_data *data = NULL;
// 	if (monster_manager_boss_free_list.empty())
// 		return NULL;
// 	ret = monster_manager_boss_free_list.back();
// 	monster_manager_boss_free_list.pop_back();
// 	data = (monster_data *)comm_pool_alloc(&monster_manager_boss_data_pool);
// 	if (!data)
// 		goto fail;
// 	memset(data, 0, sizeof(monster_data));
// 	ret->data = data;
// 	monster_manager_boss_used_list.insert(ret);
// 	ret->init_monster();
// 	return ret;
// fail:
// 	if (ret) {
// 		monster_manager_boss_used_list.erase(ret);
// 		monster_manager_boss_free_list.push_back(ret);
// 	}
// 	if (data) {
// 		comm_pool_free(&monster_manager_monster_data_pool, data);
// 	}
// 	return NULL;
// }

// void monster_manager::delete_boss_impl(boss_struct *p)
// {
// 	LOG_DEBUG("%s %d: monster[%p] data[%p]", __FUNCTION__, __LINE__, p, p->data);

// 	// if (p->data && p->data->owner)
// 	// {
// 	// 	player_struct *owner = player_manager::get_player_by_id(p->data->owner);
// 	// 	if (owner)
// 	// 	{
// 	// 		owner->del_pet(p);
// 	// 	}
// 	// }

// 	monster_manager_boss_used_list.erase(p);
// 	monster_manager_boss_free_list.push_back(p);

// 	// p->clear_all_buffs();
// 	// if (is_node_in_heap(&monster_manager_m_boss_minheap, p))
// 	// 	boss_ontick_delete(p);

// 	if (p->data) {
// 		LOG_INFO("[%s:%d] monster_id[%u], uuid[%lu]", __FUNCTION__, __LINE__, p->data->monster_id, p->data->player_id);
// 		remove_boss(p);
// 		comm_pool_free(&monster_manager_boss_data_pool, p->data);
// 		p->data = NULL;
// 	}
// }
int monster_manager::add_world_boss_monster()
{
	uint32_t cd =0;
	if(!check_active_open(WORD_BOSS_ACTIVE_ID, cd))
	{
		return -1;
	}
	struct tm tm;
	time_t now_time = time_helper::get_cached_time() / 1000;
	localtime_r(&now_time, &tm);
	//LOG_INFO("打印当前时间,时[%d] 分[%d] 秒[%d]",tm.tm_hour, tm.tm_min, tm.tm_sec)
	for(std::map<uint64_t, WorldBossTable*>::iterator ite = world_boss_config.begin(); ite != world_boss_config.end(); ite++)
	{
		for(uint32_t i = 0; i < ite->second->n_Time; i++)
		{
			tm.tm_hour = ite->second->Time[i] / 100;
			tm.tm_min = ite->second->Time[i] % 100;
			tm.tm_sec = 0;
			uint64_t st = mktime(&tm); 
			/*if(i == 0 && ite->second->ID == 510100001)
			{
				LOG_INFO("打印刷怪时间,时[%d] 分[%d] 秒[%d]",tm.tm_hour, tm.tm_min, tm.tm_sec)
				LOG_INFO("刷怪时间[%lu], 当前时间[%lu]", st, time_helper::get_cached_time() / 1000);
			}*/
			if(st == time_helper::get_cached_time() / 1000 /*&& st < time_helper::get_cached_time() / 1000 + 30*/)
			{
				//到了刷新时间点，上轮刷出的世界boss还没死，就重置
				monster_struct *monster = monster_manager::get_world_boss_by_id(ite->second->MonsterID);
			
				if(monster != NULL && monster->data != NULL)
				{
					if(monster->data->scene_id != ite->second->SceneID)
					{
						LOG_ERR("[%s:%d] 世界boss的怪物id应该是唯一的,现已经刷出的场景ID[%u],实际应该刷出的场景ID[%lu]怪物id[%u]", __FUNCTION__,__LINE__, monster->data->scene_id, ite->second->SceneID, monster->data->monster_id);
						return -3;
					}
					if( monster->data->birth_time == time_helper::get_cached_time() / 1000)
					{
						LOG_DEBUG("[%s:%d] 当前刷新点已经刷过此世界boss，不再重复刷", __FUNCTION__, __LINE__);
						return -2;
					}
					if(monster->data->attrData[PLAYER_ATTR_HP] >0)
					{
						monster->scene->delete_monster_from_scene(monster, true);
					}
					monster->data->birth_time = time_helper::get_cached_time() / 1000;
					monster->target = NULL;
					monster->area = NULL;
					monster->ai_state = AI_PATROL_STATE;
					monster->set_pos(monster->get_born_pos_x(),	monster->get_born_pos_z());
					monster->data->attrData[PLAYER_ATTR_HP] = monster->data->attrData[PLAYER_ATTR_MAXHP];
					monster->on_relive();
				}
				else
				{
					scene_struct *scene = scene_manager::get_scene(ite->second->SceneID);
					if(scene == NULL)
					{
						LOG_ERR("[%s:%d] 世界boss刷新获取场景失败", __FUNCTION__, __LINE__);
						return -4;
					}
					create_monster_by_id(scene, ite->second->MonsterID, 1, 0);
				}
				//提醒rank服更新数据
				monster = monster_manager::get_world_boss_by_id(ite->second->MonsterID);
				if(monster != NULL && monster->data != NULL && monster->config != NULL && monster->config->Type == 5 && monster->data->scene_id == ite->second->SceneID)
				{
					RankWorldBossHpInfo info;
					rank_world_boss_hp_info__init(&info);
					info.bossid = ite->second->ID;
					info.max_hp = monster->get_attr(PLAYER_ATTR_MAXHP);
					info.cur_hp = monster->get_attr(PLAYER_ATTR_HP);	
					EXTERN_DATA extern_data;
					fast_send_msg(&conn_node_gamesrv::connecter, &extern_data, SERVER_PROTO_WORLDBOSS_BIRTH_UPDATA_REDIS_INFO, rank_world_boss_hp_info__pack, info);
				}

				//世界boss刷新公告提示
				NoticeTable *table = get_config_by_id(330510010, &notify_config);
				if(table == NULL)
					return -3;
				SceneResTable *scen_config = get_config_by_id(ite->second->SceneID,&scene_res_config);
				if(scen_config == NULL)
					return -4;

				char buff[512];
				ChatHorse send;
				chat_horse__init(&send);
				send.id = 330510010;
				send.prior = table->Priority;
				send.content = buff;
				uint32_t c[MAX_CHANNEL] = { 1,2,3,4,5,6 };
				for (uint32_t i = 0; i < table->n_NoticeChannel; ++i)
				{
					c[i] = table->NoticeChannel[i];
				}
				send.channel = c;
				send.n_channel = table->n_NoticeChannel;

				snprintf(buff, 510, table->NoticeTxt, scen_config->SceneName, ite->second->Name);
				conn_node_gamesrv::send_to_all_player(MSG_ID_CHAT_HORSE_NOTIFY, &send, (pack_func)chat_horse__pack);

				RankWorldBossRefreshNotify refresh_noty;
				rank_world_boss_refresh_notify__init(&refresh_noty);
				refresh_noty.bossid = ite->second->ID;;
				conn_node_gamesrv::send_to_all_player(MSG_ID_WORLDBOSS_REFRESH_NOTIFY, &refresh_noty, (pack_func)rank_world_boss_refresh_notify__pack);
			}
		}
	}
	return 0;
}
monster_struct * monster_manager::get_world_boss_by_id(uint64_t id)
{
	std::map<uint64_t, monster_struct *>::iterator it = world_boss_all_monsters_id.find(id);
	if (it != world_boss_all_monsters_id.end())
	{
		if (it->second->mark_delete)
		{
			LOG_INFO("%s: monster[%lu] already mark delete", __FUNCTION__, id);
			return NULL;
		}
		return it->second;
	}
	
	return NULL;
}
