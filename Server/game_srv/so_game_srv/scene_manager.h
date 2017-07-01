#ifndef SCENE_MANAGER_H
#define SCENE_MANAGER_H

#include "scene.h"
#include <stdint.h>
#include <map>

extern std::map<uint64_t, scene_struct *> scene_manager_scene_map;
extern std::map<uint64_t, scene_struct *> scene_manager_guild_scene_map;
extern uint32_t scene_manager_guild_scene_id;

class scene_manager
{
public:
	static int add_scene(uint64_t id);	
	static scene_struct *get_scene(uint64_t id);

	static void set_guild_scene_id(uint64_t id);
	static int del_guild_scene(uint32_t guild_id);	
	static scene_struct *get_guild_scene(uint32_t guild_id);
private:
	static int add_guild_scene(uint32_t guild_id);	
};


#endif /* SCENE_MANAGER_H */
