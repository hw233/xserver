#include "scene_manager.h"
#include "game_event.h"

int scene_manager::add_scene(uint64_t id)
{
	LOG_INFO("%s %lu", __FUNCTION__, id);
		//todo check duplicate
	scene_struct *p = new scene_struct();
	if (!p) {
		LOG_ERR("%s %d: failed\n", __FUNCTION__, __LINE__);
		return (-1);
	}
	p->init_scene_struct(id, true);
	
//	p->id = id;
//	p->map_config = g_config_data.mapSetData.getDataById(id);

	scene_manager_scene_map[id] = p;
	return (0);
}

scene_struct *scene_manager::get_scene(uint64_t id)
{
	std::map<uint64_t, scene_struct *>::iterator it = scene_manager_scene_map.find(id);
	if (it != scene_manager_scene_map.end())
		return it->second;
	return NULL;	
}


void scene_manager::set_guild_scene_id(uint64_t id)
{
	scene_manager_guild_scene_id = id;
}

int scene_manager::add_guild_scene(uint32_t guild_id)
{
	if (guild_id == 0)
	{
		return 0;
	}
	LOG_INFO("[%s:%d] guild_id:%u", __FUNCTION__, __LINE__, guild_id);

	scene_struct *p = new scene_struct();
	if (!p) {
		LOG_ERR("%s %d: failed\n", __FUNCTION__, __LINE__);
		return (-1);
	}
	if (p->init_scene_struct(scene_manager_guild_scene_id, true))
	{
		delete p;
		LOG_ERR("[%s:%d] init failed, guild_id:%u, scene_id:%u", __FUNCTION__, __LINE__, guild_id, scene_manager_guild_scene_id);
		return (-1);
	}
	
	scene_manager_guild_scene_map[guild_id] = p;
	return (0);
}

int scene_manager::del_guild_scene(uint32_t guild_id)
{
	if (guild_id == 0)
	{
		return 0;
	}

	std::map<uint64_t, scene_struct *>::iterator it = scene_manager_guild_scene_map.find(guild_id);
	if (it != scene_manager_guild_scene_map.end())
	{
		delete it->second;
		scene_manager_guild_scene_map.erase(it);
	}
	return (0);
}

scene_struct *scene_manager::get_guild_scene(uint32_t guild_id)
{
	if (guild_id == 0)
	{
		return NULL;
	}

	std::map<uint64_t, scene_struct *>::iterator it = scene_manager_guild_scene_map.find(guild_id);
	if (it != scene_manager_guild_scene_map.end())
		return it->second;

	add_guild_scene(guild_id);
	it = scene_manager_guild_scene_map.find(guild_id);
	if (it != scene_manager_guild_scene_map.end())
		return it->second;

	return NULL;	
}


// std::map<uint64_t, scene_struct *> scene_manager::scene_manager_scene_map;
// std::map<uint64_t, scene_struct *> scene_manager::scene_manager_guild_scene_map;
// uint32_t scene_manager::scene_manager_guild_scene_id;

