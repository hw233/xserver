#include "guild_land_raid_manager.h"
#include "player_manager.h"
#include "game_event.h"
#include "uuid.h"

int guild_land_raid_manager::add_guild_land_raid(guild_land_raid_struct *p)
{
	guild_land_raid_manager_all_raid_id[p->data->uuid] = p;
	guild_land_raid_manager_raid_map[p->GUILD_LAND_DATA.guild_id] = p;
	return (0);
}

int guild_land_raid_manager::remove_guild_land_raid(guild_land_raid_struct *p)
{
	guild_land_raid_manager_all_raid_id.erase(p->data->uuid);
	guild_land_raid_manager_raid_map.erase(p->GUILD_LAND_DATA.guild_id);
	return (0);
}

guild_land_raid_struct *guild_land_raid_manager::alloc_guild_land_raid()
{
	guild_land_raid_struct *ret = NULL;
	raid_data *data = NULL;
	if (guild_land_raid_manager_raid_free_list.empty())
		return NULL;
	ret = guild_land_raid_manager_raid_free_list.back();
	guild_land_raid_manager_raid_free_list.pop_back();
	data = (raid_data *)comm_pool_alloc(&guild_land_raid_manager_raid_data_pool);
	if (!data)
		goto fail;
	memset(data, 0, sizeof(raid_data));
	ret->data = data;
	guild_land_raid_manager_raid_used_list.insert(ret);

	return ret;
fail:
	if (ret) {
		guild_land_raid_manager_raid_used_list.erase(ret);
		guild_land_raid_manager_raid_free_list.push_back(ret);
	}
	if (data) {
		comm_pool_free(&guild_land_raid_manager_raid_data_pool, data);
	}
	return NULL;
}

void guild_land_raid_manager::delete_guild_land_raid(guild_land_raid_struct *p)
{
	assert(p->m_config->DengeonRank == DUNGEON_TYPE_GUILD_LAND);		
	guild_land_raid_manager_raid_used_list.erase(p);
	guild_land_raid_manager_raid_free_list.push_back(p);

	p->clear();

	LOG_DEBUG("[%s:%d] raid[%u %lu], %p, data:%p", __FUNCTION__, __LINE__, p->m_id, p->data->uuid, p, p->data);

	if (p->data) {
		remove_guild_land_raid(p);
		comm_pool_free(&guild_land_raid_manager_raid_data_pool, p->data);
		p->data = NULL;
	}
}

void guild_land_raid_manager::delete_guild_land_raid_by_guild_id(uint32_t guild_id)
{
	guild_land_raid_struct *p = get_guild_land_raid(guild_id);
	if (p)
	{
		delete_guild_land_raid(p);
	}
}

guild_land_raid_struct *guild_land_raid_manager::get_guild_land_raid(uint32_t guild_id)
{
	std::map<uint32_t, guild_land_raid_struct *>::iterator ite = guild_land_raid_manager_raid_map.find(guild_id);
	if (ite != guild_land_raid_manager_raid_map.end())
		return ite->second;
	
	return NULL;
}

guild_land_raid_struct * guild_land_raid_manager::get_guild_land_raid_by_uuid(uint64_t id)
{
	std::map<uint64_t, guild_land_raid_struct *>::iterator it = guild_land_raid_manager_all_raid_id.find(id);
	if (it != guild_land_raid_manager_all_raid_id.end())
		return it->second;
	return NULL;	
}

guild_land_raid_struct *guild_land_raid_manager::create_guild_land_raid(uint32_t guild_id)
{
	guild_land_raid_struct *ret = alloc_guild_land_raid();
	if (!ret)
		return NULL;
	ret->data->uuid = alloc_raid_uuid();
	ret->m_id = GUILD_LAND_RAID_ID;
	ret->init_raid(NULL);

	ret->GUILD_LAND_DATA.guild_id = guild_id;

	add_guild_land_raid(ret);
	return ret;
}

int guild_land_raid_manager::init_guild_land_raid_struct(int num, unsigned long key)
{
	guild_land_raid_struct *raid;
	for (int i = 0; i < num; ++i) {
		raid = new guild_land_raid_struct();
		guild_land_raid_manager_raid_free_list.push_back(raid);
	}
	LOG_DEBUG("%s: init mem[%d][%d]", __FUNCTION__, sizeof(guild_land_raid_struct) * num, sizeof(raid_data) * num);	
	return init_comm_pool(0, sizeof(raid_data), num, key, &guild_land_raid_manager_raid_data_pool);
}

void guild_land_raid_manager::on_tick_10()
{
	
	for(std::map<uint64_t, guild_land_raid_struct *>::iterator itr =  guild_land_raid_manager_all_raid_id.begin(); itr != guild_land_raid_manager_all_raid_id.end(); itr++)			
	{
		itr->second->on_tick();

	}
}

unsigned int guild_land_raid_manager::get_guild_land_raid_pool_max_num()
{
	return guild_land_raid_manager_raid_data_pool.num;
}

