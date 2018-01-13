#ifndef CASH_TRUCK_H
#define CASH_TRUCK_H

#include <stdint.h>
#include <vector>
#include "player.h"
#include "game_config.h"


#define MAX_PLAYER_IN_CASH_TRUCK_SIGHT 30
#define MAX_MONSTER_IN_CASH_TRUCK_SIGHT 10
#define MAX_PARTNER_IN_CASH_TRUCK_SIGHT 30
#define MAX_CASH_TRUCK_SPEED_UP 10

struct cash_truck_data
{
	uint64_t player_id;
	uint32_t monster_id;
	uint32_t scene_id;  //场景ID 
//	uint32_t guild_id;  //帮会ID 
//	uint64_t raid_uuid; //副本唯一ID
	double attrData[PLAYER_ATTR_FIGHT_MAX]; //战斗属性
	double buff_fight_attr[MAX_BUFF_FIGHT_ATTR]; //战斗算上buff百分比属性

//移动路径
	struct unit_path move_path;
	float pos_y;  //注意y是高度

	uint64_t sight_player[MAX_PLAYER_IN_CASH_TRUCK_SIGHT];
	int cur_sight_player;

	uint64_t sight_monster[MAX_MONSTER_IN_CASH_TRUCK_SIGHT];
	int cur_sight_monster;

	uint64_t sight_partner[MAX_PARTNER_IN_CASH_TRUCK_SIGHT];
	int cur_sight_partner;

	uint64_t speed_up[MAX_CASH_TRUCK_SPEED_UP];
	int n_speed_up;
	uint64_t speed_reduce;
	uint64_t next_add_endruance;

	int heap_index;
	uint64_t ontick_time;

	uint8_t camp_id;
	uint32_t endurance;

	uint64_t fb_time; //触发位面副本的时间
	uint64_t owner;  //主人

};


class cash_truck_struct: public unit_struct
{
public:
	UNIT_TYPE get_unit_type();	
	bool is_avaliable();
	uint64_t get_uuid();
	double *get_all_attr();
	double get_attr(uint32_t id);
	double *get_all_buff_fight_attr();
	double get_buff_fight_attr(uint32_t id);
	uint32_t get_skill_id();
	void clear_cur_skill();
	player_struct *get_owner();	

	bool can_beattack();
	bool is_in_safe_region();
	int get_camp_id();
	void set_camp_id(int id);	
	Team *get_team();
//	int get_pk_type();
	
	
	void set_attr(uint32_t id, double value);
	struct unit_path *get_unit_path();
	float get_speed();
	int *get_cur_sight_player();
	uint64_t *get_all_sight_player();
	int *get_cur_sight_monster();
	uint64_t *get_all_sight_monster();
	virtual int *get_cur_sight_truck();
	virtual uint64_t *get_all_sight_truck();
	virtual int *get_cur_sight_partner();
	virtual uint64_t *get_all_sight_partner();
	
	JobDefine get_job();
	int get_truck_type() { return truck_config->Type; }
	
	void calculate_attribute(void);
	virtual ~cash_truck_struct();
	virtual void init_cash_truck();
	virtual void on_tick();
	void on_dead(unit_struct *killer);
	void on_hp_changed(int damage);
	void on_beattack(unit_struct *player, uint32_t skill_id, int32_t damage);
	void on_repel(unit_struct *player);
	void do_taunt_action();


	void pack_sight_cash_truck_info(SightCashTruckInfo *info);
	int broadcast_cash_truck_delete();
	int broadcast_cash_truck_create();
	int broadcast_cash_truck_move();

	void clear_cash_truck_sight();
	void update_sight(area_struct *old_area, area_struct *new_area);
	int prepare_add_player_to_sight(player_struct *player);
	int prepare_add_monster_to_sight(monster_struct *monster);
	int prepare_add_partner_to_sight(partner_struct *partner);		

	bool on_truck_leave_sight(uint64_t player_id);		
	bool on_truck_enter_sight(uint64_t player_id);		
	bool on_player_leave_sight(uint64_t player_id);
	bool on_player_enter_sight(uint64_t player_id);
	bool on_monster_leave_sight(uint64_t uuid);
	bool on_monster_enter_sight(uint64_t uuid);
	bool on_partner_leave_sight(uint64_t uuid);
	bool on_partner_enter_sight(uint64_t uuid);

	int del_partner_from_sight_both(partner_struct *partner);
	int add_partner_to_sight_both(partner_struct *partner);			

	void add_sight_space_player_to_sight(sight_space_struct *sight_space, uint16_t *add_player_id_index, uint64_t *add_player);
	void add_sight_space_monster_to_sight(sight_space_struct *sight_space);
	
//移动路径
	bool can_see_player(player_struct *player);	
	struct cash_truck_data *data;

	struct MonsterTable *config;

	struct BiaocheTable *truck_config;

	uint32_t drop_id;
	sight_space_struct *sight_space;
//	uint64_t m_liveTime;
private:
	void del_sight_player_in_area(int n_del, area_struct **del_area, int *delete_player_id_index, uint64_t *delete_player_id);
	void del_sight_monster_in_area(int n_del, area_struct **del_area);
	void del_sight_partner_in_area(int n_del, area_struct **del_area);	
	
//	int del_monster_from_sight_both(monster_struct *monster);
//	int add_monster_to_sight_both(monster_struct *monster);
	
	void add_area_player_to_sight(area_struct *area, uint16_t *add_player_id_index, uint64_t *add_player);
	void add_area_monster_to_sight(area_struct *area);
	void add_area_partner_to_sight(area_struct *area);
	
};

#endif /* CASH_TRUCK_H */
