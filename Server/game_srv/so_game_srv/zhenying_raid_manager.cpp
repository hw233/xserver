#include "zhenying_raid_manager.h"
#include "player_manager.h"
#include "game_event.h"
#include "uuid.h"
#include "zhenying_battle.h"
#include "../proto/zhenying.pb-c.h"

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
#ifdef __RAID_SRV__
	if (!ret)
		goto fail;	
	memset(ret->data, 0, sizeof(raid_data));	
#else	
	data = (raid_data *)comm_pool_alloc(&zhenying_raid_manager_raid_data_pool);
	if (!data)
		goto fail;
	memset(data, 0, sizeof(raid_data));
	ret->data = data;
#endif	
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
#ifdef __RAID_SRV__
#else				
		comm_pool_free(&zhenying_raid_manager_raid_data_pool, p->data);
		p->data = NULL;
#endif		
	}
}

zhenying_raid_struct *zhenying_raid_manager::get_avaliable_zhenying_raid(uint32_t raid_id, player_struct *player)
{
	for (std::set<zhenying_raid_struct *>::iterator iter = zhenying_raid_manager_raid_used_list.begin(); iter != zhenying_raid_manager_raid_used_list.end(); ++iter)
	{
		if ((*iter)->data->state != RAID_STATE_START)
			continue;
		if ((*iter)->data->ID != raid_id)
		{
			continue;
		}
		if ((*iter)->get_cur_player_num() < MAX_ZHENYING_RAID_PLAYER_NUM)
		{
			return (*iter);
		}
	}
	LOG_DEBUG("[%s:%d] not find %d", __FUNCTION__, __LINE__, raid_id);
	//std::vector<struct FactionBattleTable*>::iterator it = zhenying_battle_config.begin();
	//for (; it != zhenying_battle_config.end(); ++it)
	//{
	//	if (player->get_attr(PLAYER_ATTR_LEVEL) >= (*it)->LowerLimitLv && player->get_attr(PLAYER_ATTR_LEVEL) <= (*it)->UpperLimitLv)
	//	{
	//		return create_zhenying_raid(ZHENYING_RAID_ID, (*it)->LowerLimitLv);
	//	}
	//}
	return NULL;
}

unsigned int zhenying_raid_manager::get_zhenying_raid_pool_max_num()
{
	return zhenying_raid_manager_raid_data_pool.num;
}

uint64_t GetOpenZhenyingDaily()
{
	//ParameterTable *table = get_config_by_id(161000339, &parameter_config);
	//if (table == NULL)
	//{
	//	return 0;
	//}
	//for (uint64_t i = 0; i < table->n_parameter1; ++i)
	//{
	//	EventCalendarTable *tableCon = get_config_by_id(table->parameter1[0], &activity_config);
	//	if (tableCon == NULL)
	//	{
	//		return 0;
	//	}
	//	ControlTable * tableEv = get_config_by_id(tableCon->RelationID, &all_control_config);
	//		if (tableEv == NULL)
	//		{
	//			return 0;
	//		}
	//	bool open = false;
	//	for (uint32_t i = 0; i < tableEv->n_OpenDay; ++i)
	//	{
	//		if (time_helper::getWeek() == tableEv->OpenDay[i])
	//		{
	//			open = true;
	//			break;
	//		}
	//	}
	//	if (!open)
	//	{
	//		return 0;
	//	}
	//	open = false;
		struct tm tm;
		time_t tmp = time_helper::get_cached_time() / 1000;
		localtime_r(&tmp, &tm);
	//	for (uint32_t i = 0; i < tableEv->n_OpenTime; ++i)
	//	{
	//		tm.tm_hour = tableEv->OpenTime[i] / 100;
	//		tm.tm_min = tableEv->OpenTime[i] % 100;
	//		tm.tm_sec = 0;
	//		uint64_t st = mktime(&tm);
	//		tm.tm_hour = tableEv->CloseTime[i] / 100;
	//		tm.tm_min = tableEv->CloseTime[i] % 100;
	//		tm.tm_sec = 59;
	//		uint64_t end = mktime(&tm);
	//		if (time_helper::get_cached_time() / 1000 >= st && time_helper::get_cached_time() / 1000 <= end)
	//		{
	//			open = true;
	//			break;
	//		}
	//	}
	//	if (!open)
	//	{
	//		return 0;
	//	}
	//	if (tableCon->ActivityValue == 4)
	//	{
	//		return 360600001;
	//	} 
	//	else
	//	{
	//		return 360600002;
	//	}
	//}
	//return 360600001;
	if (tm.tm_wday == 2 || tm.tm_wday == 4 || tm.tm_wday == 6)
	{
		return 360600002;
	}
	else
	{
		return 360600001;
	}
}

void zhenying_raid_manager::GetRelivePos(FactionBattleTable *table, int zhenying, int *x, int *z, double *direct)
{
	if (zhenying == 1)
	{
		*x = table->BirthPoint1[0];
		*z = table->BirthPoint1[2];
		*direct = (int64_t)(table->BirthPoint1[3]);
	}
	else
	{
		*x = table->BirthPoint2[0];
		*z = table->BirthPoint2[2];
		*direct = (int64_t)(table->BirthPoint2[3]);
	}
	*x += 2 - random() % 5;
	*z += 2 - random() % 5;
}

zhenying_raid_struct *zhenying_raid_manager::add_player_to_zhenying_raid(player_struct *player)
{
	FactionBattleTable *table = get_zhenying_battle_table(player->get_attr(PLAYER_ATTR_LEVEL));
	if (table == NULL)
	{
		return NULL;
	}
	zhenying_raid_struct *ret = get_avaliable_zhenying_raid(table->Map, player);
	if (ret)
	{
		//ret->data->ai_data.zhenying_data.camp = GetOpenZhenyingDaily();
		//ret->data->ai_data.zhenying_data.lv = table->LowerLimitLv;
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

zhenying_raid_struct *zhenying_raid_manager::create_zhenying_raid(uint32_t raid_id, uint32_t lv)
{
	zhenying_raid_struct *ret = alloc_zhenying_raid();
	if (!ret)
		return NULL;
	ret->data->uuid = alloc_raid_uuid();
	ret->m_id = raid_id;
	ret->data->ai_data.zhenying_data.camp = GetOpenZhenyingDaily();
	ret->data->ai_data.zhenying_data.lv = lv;
	ret->data->ai_data.zhenying_data.progress = DAILY__MINE_STATE_REST;
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
#ifdef __RAID_SRV__
		raid->data = (raid_data *)malloc(sizeof(raid_data));
#endif		
	}
#ifdef __RAID_SRV__
	return (0);
#else	
	LOG_DEBUG("%s: init mem[%lu][%lu]", __FUNCTION__, sizeof(zhenying_raid_struct) * num, sizeof(raid_data) * num);	
	return init_comm_pool(0, sizeof(raid_data), num, key, &zhenying_raid_manager_raid_data_pool);
#endif	
}

void zhenying_raid_manager::on_tick_10()
{
	for (std::set<zhenying_raid_struct *>::iterator iter = zhenying_raid_manager_raid_used_list.begin(); iter != zhenying_raid_manager_raid_used_list.end(); )
	{
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
		for (int i = 1; i <= MAX_BATTLE_LINE_NUM; ++i)
		{
			zhenying_raid_struct *raid = zhenying_raid_manager::create_zhenying_raid((*it)->Map, (*it)->LowerLimitLv);
			raid->set_line_num(i);
			//raid->data->ai_data.zhenying_data.progress = 4;
			//raid->data->ai_data.zhenying_data.camp = GetOpenZhenyingDaily();
			//raid->data->ai_data.zhenying_data.lv = (*it)->LowerLimitLv;
			//LOG_DEBUG("%s: %s", __FUNCTION__, "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
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
