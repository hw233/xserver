#ifndef __GUILD_LAND_ACTIVE_MANAGER_H__ 
#define __GUILD_LAND_ACTIVE_MANAGER_H__ 

#include <stdint.h>

#define GUILD_BONFIRE_ACTIVITY_ID 330000043
#define GUILD_RUQIN_ACTIVITY_ID 330000038
class guild_land_raid_struct;

class guild_land_active_manager 
{
	public:
		static void on_tick_10();
		static void guild_ruqin_active_open();
		static void guild_ruqin_active_stop();
		static int guild_bonfire_open(uint32_t guild_id, bool bRepeat);
		static int guild_bonfire_close(guild_land_raid_struct *raid);
};



#endif 

