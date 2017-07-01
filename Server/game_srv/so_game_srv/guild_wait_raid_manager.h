#ifndef _GUILD_WAIT_RAID_MANAGER_H__
#define _GUILD_WAIT_RAID_MANAGER_H__

#include "mem_pool.h"
#include "guild_wait_raid.h"
#include <stdint.h>
#include <map>
#include <set>
#include <list>

#define GUILD_WAIT_RAID_ID 20029
extern std::map<uint64_t, guild_wait_raid_struct *> guild_wait_raid_manager_all_raid_id;
extern std::list<guild_wait_raid_struct *> guild_wait_raid_manager_raid_free_list;
extern std::set<guild_wait_raid_struct *> guild_wait_raid_manager_raid_used_list;
extern struct comm_pool guild_wait_raid_manager_raid_data_pool;
extern std::map<uint32_t, guild_wait_raid_struct *> guild_wait_raid_manager_raid_map;

class guild_wait_raid_manager
{
public:
	static void on_tick_10();
	static void delete_guild_wait_raid(guild_wait_raid_struct *p);
	static guild_wait_raid_struct *create_guild_wait_raid(uint32_t raid_id, uint32_t guild_id);
	static int init_guild_wait_raid_struct(int num, unsigned long key);
	static guild_wait_raid_struct * get_guild_wait_raid_by_uuid(uint64_t id);

	static guild_wait_raid_struct *add_player_to_guild_wait_raid(player_struct *player); //把玩家加入帮会等待副本
	static guild_wait_raid_struct *get_avaliable_guild_wait_raid(uint32_t guild_id);
	static guild_wait_raid_struct *get_guild_wait_raid(uint32_t guild_id);

	static unsigned int get_guild_wait_raid_pool_max_num();	
private:
	static int add_guild_wait_raid(guild_wait_raid_struct *p);
	static int remove_guild_wait_raid(guild_wait_raid_struct *p);
	static guild_wait_raid_struct *alloc_guild_wait_raid();		
};



#endif /* _GUILD_WAIT_RAID_MANAGER_H__ */
