#include "guild_wait_raid_manager.h"
#include "player_manager.h"
#include "game_event.h"
#include "uuid.h"

// std::map<uint64_t, guild_wait_raid_struct *> guild_wait_raid_manager::guild_wait_raid_manager_all_raid_id;
// std::list<guild_wait_raid_struct *> guild_wait_raid_manager::guild_wait_raid_manager_raid_free_list;
// std::set<guild_wait_raid_struct *> guild_wait_raid_manager::guild_wait_raid_manager_raid_used_list;
// struct comm_pool guild_wait_raid_manager::guild_wait_raid_manager_raid_data_pool;
// std::map<uint32_t, guild_wait_raid_struct *> guild_wait_raid_manager::guild_wait_raid_manager_raid_map;

int guild_wait_raid_manager::add_guild_wait_raid(guild_wait_raid_struct *p)
{
	guild_wait_raid_manager_all_raid_id[p->data->uuid] = p;
	guild_wait_raid_manager_raid_map[p->GUILD_WAIT_DATA.guild_id] = p;
	return (0);
}

int guild_wait_raid_manager::remove_guild_wait_raid(guild_wait_raid_struct *p)
{
	guild_wait_raid_manager_all_raid_id.erase(p->data->uuid);
	guild_wait_raid_manager_raid_map.erase(p->GUILD_WAIT_DATA.guild_id);
	return (0);
}

guild_wait_raid_struct *guild_wait_raid_manager::alloc_guild_wait_raid()
{
	guild_wait_raid_struct *ret = NULL;
	raid_data *data = NULL;
	if (guild_wait_raid_manager_raid_free_list.empty())
		return NULL;
	ret = guild_wait_raid_manager_raid_free_list.back();
	guild_wait_raid_manager_raid_free_list.pop_back();
	data = (raid_data *)comm_pool_alloc(&guild_wait_raid_manager_raid_data_pool);
	if (!data)
		goto fail;
	memset(data, 0, sizeof(raid_data));
	ret->data = data;
	guild_wait_raid_manager_raid_used_list.insert(ret);

	return ret;
fail:
	if (ret) {
		guild_wait_raid_manager_raid_used_list.erase(ret);
		guild_wait_raid_manager_raid_free_list.push_back(ret);
	}
	if (data) {
		comm_pool_free(&guild_wait_raid_manager_raid_data_pool, data);
	}
	return NULL;
}

void guild_wait_raid_manager::delete_guild_wait_raid(guild_wait_raid_struct *p)
{
	assert(p->m_config->DengeonRank == DUNGEON_TYPE_GUILD_WAIT);		
	guild_wait_raid_manager_raid_used_list.erase(p);
	guild_wait_raid_manager_raid_free_list.push_back(p);

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
		remove_guild_wait_raid(p);
		comm_pool_free(&guild_wait_raid_manager_raid_data_pool, p->data);
		p->data = NULL;
	}
}

guild_wait_raid_struct *guild_wait_raid_manager::get_avaliable_guild_wait_raid(uint32_t guild_id)
{
	std::map<uint32_t, guild_wait_raid_struct *>::iterator ite = guild_wait_raid_manager_raid_map.find(guild_id);
	if (ite != guild_wait_raid_manager_raid_map.end())
		return ite->second;
	
	return create_guild_wait_raid(GUILD_WAIT_RAID_ID, guild_id);	
}

guild_wait_raid_struct *guild_wait_raid_manager::get_guild_wait_raid(uint32_t guild_id)
{
	std::map<uint32_t, guild_wait_raid_struct *>::iterator ite = guild_wait_raid_manager_raid_map.find(guild_id);
	if (ite != guild_wait_raid_manager_raid_map.end())
		return ite->second;
	
	return NULL;
}

guild_wait_raid_struct *guild_wait_raid_manager::add_player_to_guild_wait_raid(player_struct *player, bool ignore_check)
{
		// TODO: 检查是否在活动时间
	assert(player->data->guild_id != 0);
//	assert(!player->m_team);
	guild_wait_raid_struct *ret = get_avaliable_guild_wait_raid(player->data->guild_id);
	if (ret)
		ret->add_player_to_guild_wait_raid(player, ignore_check);
	return ret;
}

guild_wait_raid_struct * guild_wait_raid_manager::get_guild_wait_raid_by_uuid(uint64_t id)
{
	std::map<uint64_t, guild_wait_raid_struct *>::iterator it = guild_wait_raid_manager_all_raid_id.find(id);
	if (it != guild_wait_raid_manager_all_raid_id.end())
		return it->second;
	return NULL;	
}

guild_wait_raid_struct *guild_wait_raid_manager::create_guild_wait_raid(uint32_t raid_id, uint32_t guild_id)
{
	guild_wait_raid_struct *ret = alloc_guild_wait_raid();
	if (!ret)
		return NULL;
	ret->data->uuid = alloc_raid_uuid();
	ret->m_id = raid_id;
	ret->init_raid(NULL);

	ret->GUILD_WAIT_DATA.guild_id = guild_id;

	add_guild_wait_raid(ret);
	return ret;
}

int guild_wait_raid_manager::init_guild_wait_raid_struct(int num, unsigned long key)
{
	guild_wait_raid_struct *raid;
	for (int i = 0; i < num; ++i) {
		raid = new guild_wait_raid_struct();
		guild_wait_raid_manager_raid_free_list.push_back(raid);
	}
	LOG_DEBUG("%s: init mem[%lu][%lu]", __FUNCTION__, sizeof(guild_wait_raid_struct) * num, sizeof(raid_data) * num);	
	return init_comm_pool(0, sizeof(raid_data), num, key, &guild_wait_raid_manager_raid_data_pool);
}

void guild_wait_raid_manager::on_tick_10()
{
}

unsigned int guild_wait_raid_manager::get_guild_wait_raid_pool_max_num()
{
	return guild_wait_raid_manager_raid_data_pool.num;
}

