#ifndef _RANK_CONFIG_H__
#define _RANK_CONFIG_H__

#include <map>
#include <vector>
#include <set>
#include <stdint.h>
#include "excel_data.h"
#include "lua_template.h"
#include "game_event.h"


extern std::map<uint64_t, struct WorldBossTable*> rank_world_boss_config; //世界boss表
int read_all_rank_excel_data();

#endif
