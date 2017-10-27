#ifndef _MULTI_RAID_H__
#define _MULTI_RAID_H__

#include "raid.h"

//多人副本
class multi_raid_struct : public raid_struct
{
public:
	virtual int init_raid(player_struct *player);
	virtual int add_player_to_scene(player_struct *player);
	virtual int delete_player_from_scene(player_struct *player);

	virtual int broadcast_to_raid(uint32_t msg_id, void *msg_data, pack_func func);
	virtual bool use_m_player();
protected:
	virtual int set_m_player_and_player_info(player_struct *player, int index);	
	virtual int clear_m_player_and_player_info(player_struct *player, bool clear_player_info);
public:
	std::map<uint64_t, player_struct *> m_players; //所有加入场景的玩家
//	std::set<uint64_t> m_players;
};

#endif /* _MULTI_RAID_H__ */
