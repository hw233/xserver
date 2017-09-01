#include "game_event.h"
#include "area.h"
#include "sortarray.h"
#include "partner_manager.h"
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#define DEFAULT_PLAYER_NUM_IN_AREA 50
#define DEFAULT_COLLECT_NUM_IN_AREA 50

int area_struct::clean_area_struct()
{
	for (int i = 0; i < cur_partner_num; ++i)
	{
		partner_struct *p = partner_manager::get_partner_by_uuid(m_partner_uuid[i]);
		if (!p)
		{
			LOG_ERR("%s: can not find partner[%lu]", __FUNCTION__, m_partner_uuid[i]);
			continue;
		}
		p->scene = NULL;
	}
	
	free(m_monster_uuid);
	free(m_collect_ids);
	free(m_player_ids);
	free(m_truck_uuid);
	free(m_partner_uuid);
	
	return (0);
}

int area_struct::init_area_struct(area_struct *base, int index, int max_x, int max_z)
{
	m_player_ids = (uint64_t *)malloc(sizeof(uint64_t) * DEFAULT_PLAYER_NUM_IN_AREA);
	cur_player_num = 0;
	max_player_num = DEFAULT_PLAYER_NUM_IN_AREA;

	m_collect_ids = (uint32_t *)malloc(sizeof(uint32_t) * DEFAULT_COLLECT_NUM_IN_AREA);
	cur_collect_num = 0;
	max_collect_num = DEFAULT_COLLECT_NUM_IN_AREA;

	cur_monster_num = 0;
	max_monster_num = 0;

	cur_truck_num = 0;
	max_truck_num = 0;

	cur_partner_num = 0;
	max_partner_num = 0;

	m_monster_uuid = NULL;
	m_truck_uuid = NULL;
	m_partner_uuid = NULL;
	
	int x = index % max_x;
	int z = index / max_x;
/*
x-1,z+1      x,z+1    x+1,z+1

x-1,z        (x,z)    x+1,z

x-1,z-1      x,z-1    x+1,z-1
 */
	if (x > 0 && z+1 < max_z)
	{
		int index = (x - 1) + (z + 1) * max_x;
		neighbour[0] = &base[index];
	}
	else
	{
		neighbour[0] = NULL;		
	}

	if (z+1 < max_z)
	{
		int index = (x) + (z + 1) * max_x;
		neighbour[1] = &base[index];
	}
	else
	{
		neighbour[1] = NULL;		
	}

	if (x+1 < max_x && z+1 < max_z)
	{
		int index = (x + 1) + (z + 1) * max_x;
		neighbour[2] = &base[index];
	}
	else
	{
		neighbour[2] = NULL;		
	}

	if (x > 0)
	{
		int index = (x - 1) + (z) * max_x;
		neighbour[3] = &base[index];
	}
	else
	{
		neighbour[3] = NULL;		
	}

	if (x+1 < max_x)
	{
		int index = (x + 1) + (z) * max_x;
		neighbour[4] = &base[index];
	}
	else
	{
		neighbour[4] = NULL;		
	}

	if (x > 0 && z > 0)
	{
		int index = (x - 1) + (z - 1) * max_x;
		neighbour[5] = &base[index];
	}
	else
	{
		neighbour[5] = NULL;		
	}

	if (z > 0)
	{
		int index = (x) + (z - 1) * max_x;
		neighbour[6] = &base[index];
	}
	else
	{
		neighbour[6] = NULL;		
	}

	if (x + 1 < max_x && z > 0)
	{
		int index = (x + 1) + (z - 1) * max_x;
		neighbour[7] = &base[index];
	}
	else
	{
		neighbour[7] = NULL;		
	}
	
	return (0);
}

bool area_struct::is_player_in_area(uint64_t player_id)
{
	int find;
	array_bsearch(&player_id, m_player_ids, cur_player_num, sizeof(uint64_t), &find, comp_uint64);
	return find;
}

void area_struct::area_neighbour_diff(area_struct *old_area, area_struct *new_area, area_struct **del, int *n_del, area_struct **add, int *n_add)
{
	*n_del = 0;
	*n_add = 0;
	bool n[MAX_NEIGHBOUR_AREA];

	for (int i = 0; i < MAX_NEIGHBOUR_AREA; ++i)
	{
		n[i] = false;
	}
	
	for (int i = 0; i < MAX_NEIGHBOUR_AREA; ++i)
	{
		if (!old_area->neighbour[i])
			continue;
		if (old_area->neighbour[i] == new_area)
			continue;
		bool find = false;
		for (int j = 0; j < MAX_NEIGHBOUR_AREA; ++j)
		{
			if (old_area->neighbour[i] != new_area->neighbour[j])
				continue;
			n[j] = true;
			find = true;
			break;
		}
		if (!find)
		{
			del[(*n_del)++] = old_area->neighbour[i];
		}
	}
	for (int i = 0; i < MAX_NEIGHBOUR_AREA; ++i)
	{
		if (n[i])
			continue;
		if (!new_area->neighbour[i])
			continue;
		if (new_area->neighbour[i] == old_area)
			continue;		
		add[(*n_add)++] = new_area->neighbour[i];
	}

		//跨越neighbour  应该是不允许的，先兼容吧
	bool find = false;	
	for (int i = 0; i < MAX_NEIGHBOUR_AREA; ++i)
	{
		if (old_area == new_area->neighbour[i])
		{
			find = true;
			break;
		}
	}
	if (!find)
	{
		del[(*n_del)++] = old_area;
		add[(*n_add)++] = new_area;		
	}
}


int area_struct::add_player_to_area(uint64_t player_id)
{
//	LOG_DEBUG("%s %d: player[%lu] [%p]", __FUNCTION__, __LINE__, player_id, this);
	if (cur_player_num == max_player_num)
	{
		m_player_ids = (uint64_t *)realloc(m_player_ids, sizeof(uint64_t) * max_player_num * 2);
		if (!m_player_ids)
		{
			LOG_ERR("%s %d: realloc fail, max_player_num = %d, err = %d", __FUNCTION__, __LINE__, max_player_num * 2, errno);
			return (-1);
		}
		max_player_num *= 2;
	}
	int ret = array_insert(&player_id, m_player_ids, &cur_player_num, sizeof(uint64_t), 1, comp_uint64);
	return ret;
}

int area_struct::del_player_from_area(uint64_t player_id)
{
//	LOG_DEBUG("%s %d: player[%lu] [%p]", __FUNCTION__, __LINE__, player_id, this);
	int ret = array_delete(&player_id, m_player_ids, &cur_player_num, sizeof(uint64_t), comp_uint64);
	return ret;
}


bool area_struct::is_monster_in_area(uint64_t uuid)
{
	int find;
	array_bsearch(&uuid, m_monster_uuid, cur_monster_num, sizeof(uint64_t), &find, comp_uint64);
	return find;
}

int area_struct::add_monster_to_area(uint64_t uuid)
{
	if (cur_monster_num == max_monster_num)
	{
		if (max_monster_num == 0)
			max_monster_num = 10;
		m_monster_uuid = (uint64_t *)realloc(m_monster_uuid, sizeof(uint64_t) * max_monster_num * 2);
		if (!m_monster_uuid)
		{
			LOG_ERR("%s %d: realloc fail, max_monster_num = %d, err = %d", __FUNCTION__, __LINE__, max_monster_num * 2, errno);
			return (-1);
		}
		max_monster_num *= 2;
	}
	int ret = array_insert(&uuid, m_monster_uuid, &cur_monster_num, sizeof(uint64_t), 1, comp_uint64);
	return ret;
}

int area_struct::del_monster_from_area(uint64_t uuid)
{
	int ret = array_delete(&uuid, m_monster_uuid, &cur_monster_num, sizeof(uint64_t), comp_uint64);
	return ret;
}

int area_struct::get_all_neighbour_player_num()
{
	int ret = 0;
	ret += cur_player_num;
	for (int i = 0; i < MAX_NEIGHBOUR_AREA; ++i)
	{
		if (neighbour[i])
			ret += neighbour[i]->cur_player_num;
	}
	return (ret);
}

bool area_struct::is_all_neighbour_have_player()
{
	if (cur_player_num > 0)
		return true;
	for (int i = 0; i < MAX_NEIGHBOUR_AREA; ++i)
	{
		if (neighbour[i] && neighbour[i]->cur_player_num > 0)
			return true;
	}
	return false;
}

int area_struct::add_collect_to_area(uint32_t collectId)
{
	if (cur_collect_num == max_collect_num)
	{
		LOG_ERR("%s %d: collect is full", __FUNCTION__, __LINE__);
		return (-1);
	}
	int ret = array_insert(&collectId, m_collect_ids, &cur_collect_num, sizeof(uint32_t), 1, comp_uint32);
	return ret;
}

int area_struct::del_collect_from_area(uint32_t collectId)
{
	int ret = array_delete(&collectId, m_collect_ids, &cur_collect_num, sizeof(uint32_t), comp_uint32);
	return ret;
}

bool area_struct::is_truck_in_area(uint64_t uuid)
{
	int find;
	array_bsearch(&uuid, m_truck_uuid, cur_truck_num, sizeof(uint64_t), &find, comp_uint64);
	return find;
}

int area_struct::add_truck_to_area(uint64_t uuid)
{
	if (cur_truck_num == max_truck_num)
	{
		if (max_truck_num == 0)
			max_truck_num = 10;
		m_truck_uuid = (uint64_t *)realloc(m_truck_uuid, sizeof(uint64_t) * max_truck_num * 2);
		if (!m_truck_uuid)
		{
			LOG_ERR("%s %d: realloc fail, max_truck_num = %d, err = %d", __FUNCTION__, __LINE__, max_truck_num * 2, errno);
			return (-1);
		}
		max_truck_num *= 2;
	}
	int ret = array_insert(&uuid, m_truck_uuid, &cur_truck_num, sizeof(uint64_t), 1, comp_uint64);
	return ret;
}

int area_struct::del_truck_from_area(uint64_t uuid)
{
	int ret = array_delete(&uuid, m_truck_uuid, &cur_truck_num, sizeof(uint64_t), comp_uint64);
	return ret;
}

bool area_struct::is_partner_in_area(uint64_t uuid)
{
	int find;
	array_bsearch(&uuid, m_partner_uuid, cur_partner_num, sizeof(uint64_t), &find, comp_uint64);
	return find;
}

int area_struct::add_partner_to_area(uint64_t uuid)
{
	if (cur_partner_num == max_partner_num)
	{
		if (max_partner_num == 0)
			max_partner_num = 10;
		m_partner_uuid = (uint64_t *)realloc(m_partner_uuid, sizeof(uint64_t) * max_partner_num * 2);
		if (!m_partner_uuid)
		{
			LOG_ERR("%s %d: realloc fail, max_partner_num = %d, err = %d", __FUNCTION__, __LINE__, max_partner_num * 2, errno);
			return (-1);
		}
		max_partner_num *= 2;
	}
	int ret = array_insert(&uuid, m_partner_uuid, &cur_partner_num, sizeof(uint64_t), 1, comp_uint64);
	return ret;
}

int area_struct::del_partner_from_area(uint64_t uuid)
{
	int ret = array_delete(&uuid, m_partner_uuid, &cur_partner_num, sizeof(uint64_t), comp_uint64);
	return ret;
}

