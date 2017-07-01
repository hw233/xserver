#ifndef RAID_MANAGER_H
#define RAID_MANAGER_H

#include "mem_pool.h"
#include "raid.h"
#include <stdint.h>
#include <map>
#include <set>
#include <list>

extern std::map<uint64_t, raid_struct *> raid_manager_all_raid_id;	
extern std::list<raid_struct *> raid_manager_raid_free_list;
extern std::set<raid_struct *> raid_manager_raid_used_list;
extern struct comm_pool raid_manager_raid_data_pool;

class raid_manager
{
public:
	static int reset_all_raid_ai();
	static void on_tick_10();
	static void delete_raid(raid_struct *p);
	static raid_struct *create_raid(uint32_t raid_id, player_struct *player);
	static int init_raid_struct(int num, unsigned long key);
	static raid_struct * get_raid_by_uuid(uint64_t id);

	static unsigned int get_raid_pool_max_num();

	static int do_team_enter_raid_cost(player_struct *player, uint32_t raid_id);	
	static int check_player_enter_raid(player_struct *player, uint32_t raid_id); //返回0表示进入副本，1表示显示准备等待界面，其他表示失败
	static int send_enter_raid_fail(player_struct *player, uint32_t err_code, uint32_t n_reason_player, uint64_t *reason_player_id, uint32_t item_id);	
	
private:
	static int do_enter_raid_cost(player_struct *player, uint32_t item_id, uint32_t item_num);	
	static int check_enter_raid_reward_time(player_struct *player, uint32_t id, struct ControlTable *config);
	static int check_enter_raid_cost(player_struct *player, struct DungeonTable *r_config);
	static int add_raid(raid_struct *p);
	static int remove_raid(raid_struct *p);
	static raid_struct *alloc_raid();
};


#endif /* RAID_MANAGER_H */
