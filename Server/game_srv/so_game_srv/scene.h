#ifndef _SCENE_H__
#define _SCENE_H__

//一个scene就是一个地图场景，整个世界由固定个数的scene和不定个数的raid组成
//一个scene里面包含多个area，每个area里面包含多个unit
//unit包括player，npc，monster，还有掉落物(droppeditem)

#include "area.h"
#include "game_config.h"
#include "map_config.h"
#include "minheap.h"
#include <string.h>
#include <vector>
#include <set>

class monster_struct;
class player_struct;
class partner_struct;
class Collect;
class unit_struct;
class cash_truck_struct;
class raid_struct;

class scene_struct
{
public:
	scene_struct();
	virtual ~scene_struct();
	virtual void clear();
	int create_all_monster(int lv);
	void clear_all_collet();
	int init_scene_struct(uint64_t sceneid, bool create_monster, int lv);
	area_struct *get_area_by_pos(float pos_x, float pos_z);
	virtual uint32_t get_area_width();
	virtual int add_monster_to_scene(monster_struct *monster, uint32_t effectid);
	virtual int add_player_to_scene(player_struct *player);
	virtual int add_collect_to_scene(Collect *pCollect);
	virtual int add_cash_truck_to_scene(cash_truck_struct *pTruck);
	virtual int add_partner_to_scene(partner_struct *partner);
	
	virtual int delete_player_from_scene(player_struct *player);
	virtual int delete_monster_from_scene(monster_struct *monster, bool send_msg);	
	virtual int delete_collect_from_scene(Collect *pCollect, bool send_msg = false);
	virtual int delete_cash_truck_from_scene(cash_truck_struct *pTruck);
	virtual int delete_partner_from_scene(partner_struct *partner, bool send_msg);
	
	void get_relive_pos(float pos_x, float pos_z, int32_t *ret_pos_x, int32_t *ret_pos_z, int32_t *ret_direct);
	virtual SCENE_TYPE_DEFINE get_scene_type();  //实际的场景类型，野外或副本
	
	virtual void on_monster_dead(monster_struct *monster, unit_struct *killer);
	virtual void on_player_dead(player_struct *player, unit_struct *killer);
	virtual void on_collect(player_struct *player, Collect *collect);
	virtual bool is_in_zhenying_raid();
	bool can_use_horse();
	bool can_transfer(uint32_t type);	
	void get_all_player(std::set<uint64_t> &playerIds);

//进入另外一个野外场景，为空表示下线	
//	virtual int enter_other_scene(player_struct *player, scene_struct *new_scene, double pos_x, double pos_y, double pos_z, double direct);
//	virtual int enter_other_raid(player_struct *player, raid_struct *new_raid);	//进入另外一个副本，为空表示下线

	virtual int player_leave_scene(player_struct *player);
	virtual int player_enter_scene(player_struct *player, double pos_x, double pos_y, double pos_z, double direct);
public:
	struct map_config *map_config;
	struct region_config *region_config;
	struct SceneResTable *res_config;
	uint32_t m_id;
	uint8_t m_area_width;
	area_struct *m_area;
		//出生点信息
	float m_born_x;
	float m_born_z;
	float m_born_y;	
	float m_born_direct;

	std::set<uint64_t> m_collect;
	std::vector<struct SceneCreateMonsterTable *> *create_monster_config;
protected:
	int broadcast_partner_create(partner_struct *partner);	
	int broadcast_monster_create(monster_struct *monster, uint32_t effectid);
	int broadcast_player_create(player_struct *player);
	int broadcast_player_delete(player_struct *player);		
	
	int calc_area_id(int pos_x, int pos_z);
	area_struct *get_area(int id);

	struct position get_area_pos(area_struct *area); //for debug

	uint16_t m_area_x_size;
	uint16_t m_area_z_size;
	uint16_t m_area_size;

	uint32_t m_guild_id;
};

extern struct minheap g_minheap;
extern struct map_block **closed_map_block;
#endif

