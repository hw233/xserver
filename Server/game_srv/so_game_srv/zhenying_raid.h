#ifndef ZHENYING_RAID_H
#define ZHENYING_RAID_H

#include "raid.h"

//日常阵营战
class zhenying_raid_struct : public raid_struct
{
public:
	bool check_raid_need_delete();
//	int init_raid(player_struct *player);
	int init_special_raid_data(player_struct *player);	
	int add_player_to_zhenying_raid(player_struct *player);
//	virtual int delete_player_from_scene(player_struct *player);
	void on_monster_dead(monster_struct *monster, unit_struct *killer);
	virtual void on_collect(player_struct *player, Collect *collect);
	virtual bool is_in_zhenying_raid();

	void create_collect();
	void delete_collect_pos(uint64_t pos);

	void set_line_num(int line) { ZHENYING_DATA.m_line = line; }
	int get_line_num() { return ZHENYING_DATA.m_line; }
	
	std::set<uint64_t> m_hit_flag;  //
protected:
	int set_m_player_and_player_info(player_struct *player, int index);	
	int clear_m_player_and_player_info(player_struct *player, bool clear_player_info);

//	int m_line;
	std::set<uint64_t> m_collect_pos;  //采集点, 宝箱位置
	
};


#endif /* ZHENYING_RAID_H */
