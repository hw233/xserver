#ifndef _GUILD_WAIT_RAID_H__
#define _GUILD_WAIT_RAID_H__

#include "raid.h"

class guild_wait_raid_struct : public raid_struct
{
public:
//	int init_raid(player_struct *player);
	int init_special_raid_data(player_struct *player);	
	void add_player_to_guild_wait_raid(player_struct *player);
	virtual int add_player_to_scene(player_struct *player);
	virtual int delete_player_from_scene(player_struct *player);
	void on_monster_dead(monster_struct *monster, unit_struct *killer);

	virtual void on_collect(player_struct *player, Collect *collect);
	virtual int broadcast_to_raid(uint32_t msg_id, void *msg_data, pack_func func);

	void get_wait_player(std::set<uint64_t> &playerIds);
protected:
	int set_m_player_and_player_info(player_struct *player, int index);	
	int clear_m_player_and_player_info(player_struct *player, bool clear_player_info);
	std::set<uint64_t> m_players;
};


#endif /* _GUILD_WAIT_RAID_H__ */
