#ifndef _GUILD_WAIT_RAID_H__
#define _GUILD_WAIT_RAID_H__

#include "multi_raid.h"

class guild_wait_raid_struct : public multi_raid_struct
{
public:
	int init_special_raid_data(player_struct *player);	
	void add_player_to_guild_wait_raid(player_struct *player, bool ignore_check);

	void on_monster_dead(monster_struct *monster, unit_struct *killer);
	virtual void on_collect(player_struct *player, Collect *collect);

	void get_wait_player(std::set<uint64_t> &playerIds);
};


#endif /* _GUILD_WAIT_RAID_H__ */
