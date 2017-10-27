#ifndef __GUILD_LAND_ACTIVE_MANAGER_H__ 
#define __GUILD_LAND_ACTIVE_MANAGER_H__ 

#include <stdint.h>
#include "team.h"
#include "time_helper.h"
#include "excel_data.h"
#include "lua_config.h"
#include "guild_land_raid_manager.h"
#include "raid_ai_common.h"
#include "guild_battle_manager.h"

class guild_land_active_manager 
{
	public:
		static void on_tick_10();
		static void guild_ruqin_active_open();
		static void guild_ruqin_active_stop();
};



#endif 

