#ifndef CASH_TRUCK_MANAGER_H
#define CASH_TRUCK_MANAGER_H

#include "scene.h"
#include "mem_pool.h"
#include "cash_truck.h"
#include "sight_space.h"
#include "server_proto.h"
#include <map>
#include <set>
#include <list>

extern std::list<cash_truck_struct *> cash_truck_manager_free_list;
extern std::set<cash_truck_struct *> cash_truck_manager_used_list;
extern struct comm_pool cash_truck_manager_data_pool;
extern std::map<uint64_t, cash_truck_struct *> cash_truck_manager_all_id;


class cash_truck_manager
{
public:
	cash_truck_manager();
	virtual ~cash_truck_manager();

//	static void on_tick_10();

//	static unsigned int get_cash_truck_pool_max_num();


	static cash_truck_struct *create_cash_truck_at_pos(scene_struct *scene, uint64_t id, player_struct &player);
	static cash_truck_struct *create_cash_truck_at_pos(scene_struct *scene, uint64_t id, player_struct &player, float x, float z, uint32_t hp);
	//static cash_truck_struct *add_cash_truck(uint64_t monster_id, uint64_t lv, unit_struct *owner = NULL);
	static void delete_cash_truck(cash_truck_struct *p);
	static cash_truck_struct * get_cash_truck_by_id(uint64_t id);

	static int init_cash_truck_struct(int num, unsigned long key);
	static int resume_cash_truck_struct(int num, unsigned long key);	

	static void cash_truck_drop(player_struct &player);

private:

	static cash_truck_struct *alloc_cash_truck();

private:
};

#endif /* CASH_TRUCK_MANAGER_H */
