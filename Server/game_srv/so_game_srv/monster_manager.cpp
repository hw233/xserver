#include "game_event.h"
#include "monster_manager.h"
#include "player_manager.h"
#include "sight_space_manager.h"
#include "mem_pool.h"
#include "conn_node_gamesrv.h"
#include "uuid.h"
#include "msgid.h"
#include "game_config.h"
#include <stdio.h>
#include <errno.h>


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
	return (0);
}

int monster_manager::remove_monster(monster_struct *p)
{
	monster_manager_all_monsters_id.erase(p->data->player_id);
	return (0);
}

monster_struct * monster_manager::get_monster_by_id(uint64_t id)
{
	std::map<uint64_t, monster_struct *>::iterator it = monster_manager_all_monsters_id.find(id);
	if (it != monster_manager_all_monsters_id.end())
	{
		if (it->second->mark_delete)
			LOG_ERR("%s: monster[%lu] already mark delete", __FUNCTION__, id);
		return it->second;
	}

	return get_boss_by_id(id);
//	return NULL;
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

	{
		std::map<uint64_t, boss_struct *>::iterator it = monster_manager_all_boss_id.begin();
		for (; it != monster_manager_all_boss_id.end(); ++it)
		{
			it->second->set_ai_interface(it->second->ai_type);
		}
	}
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
	push_heap(&monster_manager_m_minheap, p);
}

void monster_manager::monster_ontick_reset_timer(monster_struct *p)
{
	assert(p->mark_delete == false);	
	adjust_heap_node(&monster_manager_m_minheap, p);
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
	erase_heap_node(&monster_manager_m_minheap, p);
}

//////////////////////////////////////////

int monster_manager::init_monster_struct(int num, unsigned long key)
{
	init_heap(&monster_manager_m_minheap, num, minheap_cmp_monster_timeout, minheap_get_monster_timeout_index, minheap_set_monster_timeout_index);

	monster_struct *monster;
	for (int i = 0; i < num; ++i) {
		monster = new monster_struct();
		monster_manager_monster_free_list.push_back(monster);
	}
	return init_comm_pool(0, sizeof(monster_data), num, key, &monster_manager_monster_data_pool);
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
	data = (monster_data *)comm_pool_alloc(&monster_manager_monster_data_pool);
	if (!data)
		goto fail;
	memset(data, 0, sizeof(monster_data));
	ret->data = data;
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
		comm_pool_free(&monster_manager_monster_data_pool, p->data);
		p->data = NULL;
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
	boss_struct *boss = get_ontick_boss(now);
	while (boss != NULL)
	{
		boss->data->ontick_time = now + boss->ai_config->Response;
		if (!boss->data->stop_ai)
			boss->on_tick();
		if (boss->data)
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
			if (monster->config->Type == 4)
			{
				player_struct *player = player_manager::get_player_by_id(monster->data->owner);
				if (player != NULL)
				{
					//系统提示
					std::vector<char *> args;
					player->send_system_notice(190500285, args);
				}
			}
			if (monster->sight_space != NULL)
			{
				sight_space_manager::mark_sight_space_delete(monster->sight_space);
				monster = get_ontick_monster(now);
				continue; //位面副本 等出位面再删
			}
			monster->scene->delete_monster_from_scene(monster, true);
			delete_monster(monster);
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
		switch ((int)(*ite)->get_unit_type())
		{
			case UNIT_TYPE_MONSTER:
				delete_monster_impl(*ite);
				break;
			case UNIT_TYPE_BOSS:
				delete_boss_impl((boss_struct *)(*ite));
				break;
		}
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
unsigned int monster_manager::get_boss_pool_max_num()
{
	return monster_manager_boss_data_pool.num;
}

monster_struct *monster_manager::add_monster(uint64_t monster_id, uint64_t lv, unit_struct *owner)
{
	std::map<uint64_t, struct MonsterTable *>::iterator ite;
	ite = monster_config.find(monster_id);
	if (ite == monster_config.end())
		return NULL;
	monster_struct *ret;

	switch (ite->second->HateType)
	{
		case MONSTER_TYPE_DEFINE_BOSS:
		{
			boss_struct *boss = alloc_boss();
			if (!boss)
				return NULL;
			boss->data->player_id = alloc_monster_uuid();
			add_boss(boss);
			ret = boss;
		}
		break;
		default:
		{
			ret = alloc_monster();
			if (!ret)
				return NULL;
			ret->data->player_id = alloc_monster_uuid();
			add_monster(ret);
		}
		break;
	}

	ret->config = ite->second;
	ret->ai_config = base_ai_config[ret->config->BaseID];
	ret->ai_type = ret->ai_config->AIType;
	ret->ai_state = AI_PATROL_STATE;
	ret->set_ai_interface(ret->ai_type);
	ret->data->monster_id = monster_id;
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
	monster_struct *monster = add_monster(lv_config->MonsterID, lv_config->MonsterLv, player);
	if (!monster)
		return NULL;
	monster->create_config = NULL;
	monster->data->create_config_index = -1;
	monster->set_pos(player->get_pos()->pos_x, player->get_pos()->pos_z);
	if (player->scene->add_monster_to_scene(monster, lv_config->MonsterEff) != 0)
	{
		LOG_ERR("%s: uuid[%lu] monster[%lu] scene[%u]", __FUNCTION__, monster->data->player_id, lv_config->MonsterID, player->scene->m_id);
		delete_monster(monster);
	}

	return monster;
}

monster_struct *monster_manager::create_sight_space_monster(sight_space_struct *sight_space, scene_struct *scene, uint32_t monster_id, uint32_t level, double pos_x, double pos_z)
{
	for (int i = 0; i < MAX_MONSTER_IN_SIGHT_SPACE; ++i)
	{
		if (sight_space->monsters[i] != NULL)
			continue;

		monster_struct *monster = add_monster(monster_id, level);
		if (!monster)
			return NULL;

		monster->create_config = NULL;
		monster->data->create_config_index = -1;
		monster->set_pos(pos_x, pos_z);
		monster->sight_space = sight_space;
		sight_space->monsters[i] = monster;
		sight_space->data->monster_uuid[i] = monster->get_uuid();

		if (monster->ai && monster->ai->on_alive)
			monster->ai->on_alive(monster);
		sight_space->broadcast_monster_create(monster);
		monster->scene = scene;
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

monster_struct *monster_manager::create_monster_at_pos(scene_struct *scene, uint64_t id, uint32_t lv, int32_t pos_x, int32_t pos_z, uint32_t effectid, unit_struct *owner)
{
	monster_struct *monster = add_monster(id, lv, owner);
	if (!monster)
		return NULL;
	monster->data->create_config_index = 0;
	monster->create_config = NULL;
	monster->set_pos(pos_x, pos_z);

	if (scene->add_monster_to_scene(monster, effectid) != 0)
	{
		LOG_ERR("%s: uuid[%lu] monster[%lu] scene[%u]", __FUNCTION__, monster->data->player_id, id, scene->m_id);
	}
	return monster;
}

int monster_manager::create_monster_by_id(scene_struct *scene, uint32_t id, uint32_t num)
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
		create_monster_by_config(scene, i);
		--num;
	}

	return (0);
}

monster_struct *monster_manager::create_monster_by_config(scene_struct *scene, int index)
{
	struct SceneCreateMonsterTable *create_config = (*scene->create_monster_config)[index];
	if (!create_config)
		return NULL;

	monster_struct *monster = add_monster(create_config->ID, create_config->Level);
	if (!monster)
		return NULL;
	monster->data->create_config_index = index;
	monster->create_config = create_config;
	switch (monster->ai_type)
	{
		case AI_TYPE_CIRCLE:
		case AI_TYPE_ESCORT:
			monster->set_pos(create_config->TargetInfoList[0]->TargetPos->TargetPosX,
				create_config->TargetInfoList[0]->TargetPos->TargetPosZ);
			break;
//		case AI_TYPE_NORMAL:
		default:
			monster->set_pos(monster->get_born_pos_x(), monster->get_born_pos_z());
			break;
	}
	if (scene->add_monster_to_scene(monster, 0) != 0)
	{
		LOG_ERR("%s: uuid[%lu] monster[%lu] scene[%u]", __FUNCTION__, monster->data->player_id, create_config->ID, scene->m_id);
	}
	return monster;
}

//////////////////////////////boss

int monster_manager::add_boss(boss_struct *p)
{
	monster_manager_all_boss_id[p->data->player_id] = p;
	return (0);
}

int monster_manager::remove_boss(boss_struct *p)
{
	monster_manager_all_boss_id.erase(p->data->player_id);
	return (0);
}

boss_struct * monster_manager::get_boss_by_id(uint64_t id)
{
	std::map<uint64_t, boss_struct *>::iterator it = monster_manager_all_boss_id.find(id);
	if (it != monster_manager_all_boss_id.end())
	{
		if (it->second->mark_delete)
			LOG_ERR("%s: boss[%lu] already mark delete", __FUNCTION__, id);		
		return it->second;
	}
	return NULL;
}

//////////////////////////////////////////////
void monster_manager::boss_ontick_settimer(boss_struct *p)
{
	push_heap(&monster_manager_m_boss_minheap, p);
}

void monster_manager::boss_ontick_reset_timer(boss_struct *p)
{
	adjust_heap_node(&monster_manager_m_boss_minheap, p);
}

boss_struct *monster_manager::get_ontick_boss(uint64_t now)
{
	if (monster_manager_m_boss_minheap.cur_size == 0)
		return NULL;

	if (((boss_struct *)get_heap_first(&monster_manager_m_boss_minheap))->data->ontick_time > now)
		return NULL;
	return (boss_struct *)pop_heap(&monster_manager_m_boss_minheap);
}

void monster_manager::boss_ontick_delete(boss_struct *p)
{
	erase_heap_node(&monster_manager_m_boss_minheap, p);
}

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

int monster_manager::init_boss_struct(int num, unsigned long key)
{
	init_heap(&monster_manager_m_boss_minheap, num, minheap_cmp_monster_timeout, minheap_get_monster_timeout_index, minheap_set_monster_timeout_index);

	boss_struct *monster;
	for (int i = 0; i < num; ++i) {
		monster = new boss_struct();
		monster_manager_boss_free_list.push_back(monster);
	}
	return init_comm_pool(0, sizeof(monster_data), num, key, &monster_manager_boss_data_pool);
}

boss_struct *monster_manager::alloc_boss()
{
	boss_struct *ret = NULL;
	monster_data *data = NULL;
	if (monster_manager_boss_free_list.empty())
		return NULL;
	ret = monster_manager_boss_free_list.back();
	monster_manager_boss_free_list.pop_back();
	data = (monster_data *)comm_pool_alloc(&monster_manager_boss_data_pool);
	if (!data)
		goto fail;
	memset(data, 0, sizeof(monster_data));
	ret->data = data;
	monster_manager_boss_used_list.insert(ret);
	ret->init_monster();
	return ret;
fail:
	if (ret) {
		monster_manager_boss_used_list.erase(ret);
		monster_manager_boss_free_list.push_back(ret);
	}
	if (data) {
		comm_pool_free(&monster_manager_monster_data_pool, data);
	}
	return NULL;
}

void monster_manager::delete_boss_impl(boss_struct *p)
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

	monster_manager_boss_used_list.erase(p);
	monster_manager_boss_free_list.push_back(p);

	// p->clear_all_buffs();
	// if (is_node_in_heap(&monster_manager_m_boss_minheap, p))
	// 	boss_ontick_delete(p);

	if (p->data) {
		LOG_INFO("[%s:%d] monster_id[%u], uuid[%lu]", __FUNCTION__, __LINE__, p->data->monster_id, p->data->player_id);
		remove_boss(p);
		comm_pool_free(&monster_manager_boss_data_pool, p->data);
		p->data = NULL;
	}
}
