#ifndef _GUILD_LAND_RAID_H__
#define _GUILD_LAND_RAID_H__

#include <string.h>
#include "multi_raid.h"


class guild_land_raid_struct : public multi_raid_struct
{
public:
	guild_land_raid_struct();
	~guild_land_raid_struct();
	int init_special_raid_data(player_struct *player);	
	int init_guild_ruqin_active_data();
	void clear_monster();
};


#endif /* _GUILD_LAND_RAID_H__ */
