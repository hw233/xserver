#ifndef ZHENYING_RAID_MANAGER_H
#define ZHENYING_RAID_MANAGER_H

#include "mem_pool.h"
#include "zhenying_raid.h"
#include <stdint.h>
#include <map>
#include <set>
#include <list>

#define ZHENYING_RAID_ID 30001
#define MAX_ZHENYING_RAID_PLAYER_NUM 2

struct FactionBattleTable;

extern std::map<uint64_t, zhenying_raid_struct *> zhenying_raid_manager_all_raid_id;
extern std::list<zhenying_raid_struct *> zhenying_raid_manager_raid_free_list;
extern std::set<zhenying_raid_struct *> zhenying_raid_manager_raid_used_list;
extern struct comm_pool zhenying_raid_manager_raid_data_pool;
extern uint64_t zhenying_raid_manager_reflesh_collect;

//日常阵营战
class zhenying_raid_manager
{
public:
	static void on_tick_10();
	static void delete_zhenying_raid(zhenying_raid_struct *p);
	static zhenying_raid_struct *create_zhenying_raid(uint32_t raid_id);
	static int init_zhenying_raid_struct(int num, unsigned long key);
	static zhenying_raid_struct * get_zhenying_raid_by_uuid(uint64_t id);
	static zhenying_raid_struct * get_zhenying_raid_by_line(uint32_t raid_id, uint32_t line);
	static void create_all_line();
	static void get_all_line_info(player_struct *player); //

	static zhenying_raid_struct *get_avaliable_zhenying_raid(uint32_t raid_id);
	static zhenying_raid_struct *add_player_to_zhenying_raid(player_struct *player); //把玩家加入阵营副本

	static unsigned int get_zhenying_raid_pool_max_num();	

	static void GetRelivePos(FactionBattleTable *table, int zhenying, int *x, int *z, double *direct);
		
private:
	static int add_zhenying_raid(zhenying_raid_struct *p);
	static int remove_zhenying_raid(zhenying_raid_struct *p);
	static zhenying_raid_struct *alloc_zhenying_raid();		
};



#endif /* ZHENYING_RAID_MANAGER_H */
