#ifndef _GUILD_LAND_RAID_MANAGER_H__
#define _GUILD_LAND_RAID_MANAGER_H__

#include "mem_pool.h"
#include "guild_land_raid.h"
#include <stdint.h>
#include <map>
#include <set>
#include <list>

#define GUILD_LAND_RAID_ID 40001
extern std::map<uint64_t, guild_land_raid_struct *> guild_land_raid_manager_all_raid_id;
extern std::list<guild_land_raid_struct *> guild_land_raid_manager_raid_free_list;
extern std::set<guild_land_raid_struct *> guild_land_raid_manager_raid_used_list;
extern struct comm_pool guild_land_raid_manager_raid_data_pool;
extern std::map<uint32_t, guild_land_raid_struct *> guild_land_raid_manager_raid_map;

class guild_land_raid_manager
{
public:
	static void on_tick_10();
	static void delete_guild_land_raid(guild_land_raid_struct *p);
	static void delete_guild_land_raid_by_guild_id(uint32_t guild_id);
	static guild_land_raid_struct *create_guild_land_raid(uint32_t guild_id);
	static int init_guild_land_raid_struct(int num, unsigned long key);
	static guild_land_raid_struct *get_guild_land_raid_by_uuid(uint64_t id);

	static guild_land_raid_struct *get_guild_land_raid(uint32_t guild_id);

	static unsigned int get_guild_land_raid_pool_max_num();	
private:
	static int add_guild_land_raid(guild_land_raid_struct *p);
	static int remove_guild_land_raid(guild_land_raid_struct *p);
	static guild_land_raid_struct *alloc_guild_land_raid();		
};



#endif /* _GUILD_LAND_RAID_MANAGER_H__ */
