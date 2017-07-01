#include "zhenying_raid_manager.h"
#include "player_manager.h"
#include "game_event.h"
#include "uuid.h"
#include "zhenying_battle.h"

// std::map<uint64_t, zhenying_raid_struct *> zhenying_raid_manager::zhenying_raid_manager_all_raid_id;
// std::list<zhenying_raid_struct *> zhenying_raid_manager::zhenying_raid_manager_raid_free_list;
// std::set<zhenying_raid_struct *> zhenying_raid_manager::zhenying_raid_manager_raid_used_list;
// struct comm_pool zhenying_raid_manager::zhenying_raid_manager_raid_data_pool;
// uint64_t zhenying_raid_manager::zhenying_raid_manager_reflesh_collect;
int zhenying_raid_manager::add_zhenying_raid(zhenying_raid_struct *p)
{
	zhenying_raid_manager_all_raid_id[p->data->uuid] = p;
	return (0);
}

int zhenying_raid_manager::remove_zhenying_raid(zhenying_raid_struct *p)
{
	zhenying_raid_manager_all_raid_id.erase(p->data->uuid);
	return (0);
}

zhenying_raid_struct *zhenying_raid_manager::alloc_zhenying_raid()
{
	zhenying_raid_struct *ret = NULL;
	raid_data *data = NULL;
	if (zhenying_raid_manager_raid_free_list.empty())
		return NULL;
	ret = zhenying_raid_manager_raid_free_list.back();
	zhenying_raid_manager_raid_free_list.pop_back();
	data = (raid_data *)comm_pool_alloc(&zhenying_raid_manager_raid_data_pool);
	if (!data)
		goto fail;
	memset(data, 0, sizeof(raid_data));
	ret->data = data;
	zhenying_raid_manager_raid_used_list.insert(ret);

	return ret;
fail:
	if (ret) {
		zhenying_raid_manager_raid_used_list.erase(ret);
		zhenying_raid_manager_raid_free_list.push_back(ret);
	}
	if (data) {
		comm_pool_free(&zhenying_raid_manager_raid_data_pool, data);
	}
	return NULL;
}

void zhenying_raid_manager::delete_zhenying_raid(zhenying_raid_struct *p)
{
	assert(p->m_config->DengeonRank == DUNGEON_TYPE_ZHENYING);	
	zhenying_raid_manager_raid_used_list.erase(p);
	zhenying_raid_manager_raid_free_list.push_back(p);

	p->clear();

	LOG_DEBUG("[%s:%d] raid[%u %lu], %p, data:%p", __FUNCTION__, __LINE__, p->m_id, p->data->uuid, p, p->data);

	// for (int i = 0; i < MAX_TEAM_MEM; ++i)
	// {
	// 	if (p->m_player[i] && p->m_player[i]->data && get_entity_type(p->m_player[i]->get_uuid()) == ENTITY_TYPE_AI_PLAYER)
	// 		player_manager::delete_player(p->m_player[i]);
	// 	if (p->m_player2[i] && p->m_player2[i]->data && get_entity_type(p->m_player2[i]->get_uuid()) == ENTITY_TYPE_AI_PLAYER)
	// 		player_manager::delete_player(p->m_player2[i]);
	// }

	if (p->data) {
		remove_zhenying_raid(p);
		comm_pool_free(&zhenying_raid_manager_raid_data_pool, p->data);
		p->data = NULL;
	}
}

zhenying_raid_struct *zhenying_raid_manager::get_avaliable_zhenying_raid()
{
	for (std::set<zhenying_raid_struct *>::iterator iter = zhenying_raid_manager_raid_used_list.begin(); iter != zhenying_raid_manager_raid_used_list.end(); ++iter)
	{
		if ((*iter)->data->state != RAID_STATE_START)
			continue;
		if ((*iter)->get_cur_player_num() < MAX_ZHENYING_RAID_PLAYER_NUM)
		{
			return (*iter);
		}
	}
	return create_zhenying_raid(ZHENYING_RAID_ID);	
}

unsigned int zhenying_raid_manager::get_zhenying_raid_pool_max_num()
{
	return zhenying_raid_manager_raid_data_pool.num;
}

zhenying_raid_struct *zhenying_raid_manager::add_player_to_zhenying_raid(player_struct *player)
{
	zhenying_raid_struct *ret = get_avaliable_zhenying_raid();
	if (ret)
	{
		if (ret->add_player_to_zhenying_raid(player) != 0)
			return NULL;
	}
	return ret;
}

zhenying_raid_struct * zhenying_raid_manager::get_zhenying_raid_by_uuid(uint64_t id)
{
	std::map<uint64_t, zhenying_raid_struct *>::iterator it = zhenying_raid_manager_all_raid_id.find(id);
	if (it != zhenying_raid_manager_all_raid_id.end())
		return it->second;
	return NULL;	
}

zhenying_raid_struct *zhenying_raid_manager::get_zhenying_raid_by_line(uint32_t raid_id, uint32_t line)
{
	zhenying_raid_struct *raid = NULL;
	std::map<uint64_t, zhenying_raid_struct *>::iterator iter = zhenying_raid_manager_all_raid_id.begin();
	for (; iter != zhenying_raid_manager_all_raid_id.end(); ++iter)
	{
		raid = iter->second;
		if (raid->m_id == raid_id && raid->get_line_num() == (int)line)
		{
			return raid;
		}
	}
	return NULL;
}

zhenying_raid_struct *zhenying_raid_manager::create_zhenying_raid(uint32_t raid_id)
{
	zhenying_raid_struct *ret = alloc_zhenying_raid();
	if (!ret)
		return NULL;
	ret->data->uuid = alloc_raid_uuid();
	ret->m_id = raid_id;
	ret->init_raid(NULL);

	add_zhenying_raid(ret);
	return ret;
}

int zhenying_raid_manager::init_zhenying_raid_struct(int num, unsigned long key)
{
	zhenying_raid_struct *raid;
	for (int i = 0; i < num; ++i) {
		raid = new zhenying_raid_struct();
		zhenying_raid_manager_raid_free_list.push_back(raid);
	}
	return init_comm_pool(0, sizeof(raid_data), num, key, &zhenying_raid_manager_raid_data_pool);
}

void zhenying_raid_manager::on_tick_10()
{
	bool reflesh = false;
	if (zhenying_raid_manager_reflesh_collect < time_helper::get_cached_time())
	{
		std::vector<struct FactionBattleTable*>::iterator it = zhenying_battle_config.begin();
		FactionBattleTable *table = *it;
		zhenying_raid_manager_reflesh_collect = time_helper::get_cached_time() + table->BoxReloadTime;
		reflesh = true;
	}
	
	for (std::set<zhenying_raid_struct *>::iterator iter = zhenying_raid_manager_raid_used_list.begin(); iter != zhenying_raid_manager_raid_used_list.end(); )
	{
		if (reflesh)
		{
			(*iter)->create_collect();
		}
		if ((*iter)->check_raid_need_delete())
		{
			zhenying_raid_struct *raid = (*iter);
			++iter;
//			if (!raid->m_team)
			delete_zhenying_raid(raid);
		}
		else
		{
			(*iter)->on_tick();
			++iter;
		}
	}

}

void zhenying_raid_manager::create_all_line()
{
	std::vector<struct FactionBattleTable*>::iterator it = zhenying_battle_config.begin();
	for (; it != zhenying_battle_config.end(); ++it)
	{
		for (int i = 1; i <= ZhenyingBattle::MAX_LINE_NUM; ++i)
		{
			zhenying_raid_struct *raid = zhenying_raid_manager::create_zhenying_raid((*it)->Map);
			raid->set_line_num(i);
		}
	}
	std::vector<struct FactionBattleTable*>::iterator itF = zhenying_battle_config.begin();
	FactionBattleTable *table = *itF;
	if (table == NULL)
	{
		return;
	}
	zhenying_raid_manager_reflesh_collect = time_helper::get_cached_time() + table->BoxReloadTime;
}

void zhenying_raid_manager::get_all_line_info(player_struct *player)
{
	

	
}
