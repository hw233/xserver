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
	p->init_scene_struct(id, true, 0);
	
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

// std::map<uint64_t, scene_struct *> scene_manager::scene_manager_scene_map;

