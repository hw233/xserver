#ifndef _AREA_H__
#define _AREA_H__
#include <stdint.h>

#define MAX_NEIGHBOUR_AREA 8

struct area_struct
{
public:
	static void area_neighbour_diff(area_struct *old_area, area_struct *new_area, area_struct **del, int *n_del, area_struct **add, int *n_add);
	
	int init_area_struct(area_struct *base, int index, int max_x, int max_z);
	int clean_area_struct();
	
	bool is_player_in_area(uint64_t player_id);
	int add_player_to_area(uint64_t player_id);
	int del_player_from_area(uint64_t player_id);	
	
	uint64_t *m_player_ids;	
	int cur_player_num;
	int max_player_num;

	bool is_monster_in_area(uint64_t uuid);
	int add_monster_to_area(uint64_t uuid);
	int del_monster_from_area(uint64_t uuid);
	int get_all_neighbour_player_num();
	bool is_all_neighbour_have_player();
	bool is_neighbour(struct area_struct *area);
	
	uint64_t *m_monster_uuid;	
	int cur_monster_num;
	int max_monster_num;
	
	
	bool is_collect_in_area(uint32_t collectId);
	int add_collect_to_area(uint32_t collectId);
	int del_collect_from_area(uint32_t collectId);

	uint32_t *m_collect_ids;
	int cur_collect_num;
	int max_collect_num;

	bool is_truck_in_area(uint64_t uuid);
	int add_truck_to_area(uint64_t uuid);
	int del_truck_from_area(uint64_t uuid);
	uint64_t *m_truck_uuid;
	int cur_truck_num;
	int max_truck_num;

	bool is_partner_in_area(uint64_t uuid);
	int add_partner_to_area(uint64_t uuid);
	int del_partner_from_area(uint64_t uuid);
	uint64_t *m_partner_uuid;
	int cur_partner_num;
	int max_partner_num;

	area_struct *neighbour[MAX_NEIGHBOUR_AREA];
};

#endif
