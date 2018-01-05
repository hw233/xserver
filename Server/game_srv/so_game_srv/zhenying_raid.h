#ifndef ZHENYING_RAID_H
#define ZHENYING_RAID_H

#include "raid.h"

//日常阵营战
class zhenying_raid_struct : public raid_struct
{
public:
	static int raid_num;
	zhenying_raid_struct();
	~zhenying_raid_struct();

	virtual bool use_m_player();	
	bool check_raid_need_delete();
//	int init_raid(player_struct *player);
	int init_special_raid_data(player_struct *player);	
	int add_player_to_zhenying_raid(player_struct *player);
//	virtual int delete_player_from_scene(player_struct *player);
	void on_monster_dead(monster_struct *monster, unit_struct *killer);
	virtual void on_collect(player_struct *player, Collect *collect);
	virtual bool is_in_zhenying_raid();

	void set_line_num(int line) { ZHENYING_DATA.m_line = line; }
	int get_line_num() { return ZHENYING_DATA.m_line; }
	
protected:
	int set_m_player_and_player_info(player_struct *player, int index);	
	int clear_m_player_and_player_info(player_struct *player, bool clear_player_info);
};


#endif /* ZHENYING_RAID_H */
