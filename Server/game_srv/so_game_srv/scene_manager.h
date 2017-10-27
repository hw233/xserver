#ifndef SCENE_MANAGER_H
#define SCENE_MANAGER_H

#include "scene.h"
#include <stdint.h>
#include <map>

extern std::map<uint64_t, scene_struct *> scene_manager_scene_map;

class scene_manager
{
public:
	static int add_scene(uint64_t id);	
	static scene_struct *get_scene(uint64_t id);
};


#endif /* SCENE_MANAGER_H */
