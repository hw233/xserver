#ifndef UNIT_H
#define UNIT_H

#include <list>
#include <stdint.h>
#include "sortarray.h"
#include "conn_node_gamesrv.h"
#include "game_event.h"
#include "unit_path.h"
#include "attr_id.h"
#include "scene.h"
#include "area.h"
#include "../proto/comm_message.pb-c.h"

enum UNIT_TYPE
{
	UNIT_TYPE_PLAYER,
	UNIT_TYPE_MONSTER,
	UNIT_TYPE_BOSS,
	UNIT_TYPE_CASH_TRUCK,
	UNIT_TYPE_PARTNER,	
};

#define MAX_BUFF_PER_UNIT (12)
class buff_struct;
class Team;
class unit_struct;

class unit_struct
{
public:
	void init_unit_struct();
	virtual bool can_beattack() = 0;
	virtual bool is_in_safe_region() = 0;
	virtual int get_camp_id() = 0;         //红蓝阵营关系, 战斗关系优先级最高, 一方为0的时候被忽略
	virtual void set_camp_id(int id) = 0;	
	virtual Team *get_team() = 0;
//	virtual int get_pk_type() = 0;
	virtual void clear_cur_skill() = 0; //嘲讽的时候打断当前技能
	virtual UNIT_TYPE get_unit_type() = 0;
	virtual uint64_t get_uuid() = 0;
	virtual double *get_all_attr() = 0;
	virtual double *get_all_buff_fight_attr() = 0;	
	virtual double get_attr(uint32_t id) = 0;
	virtual double get_buff_fight_attr(uint32_t id) = 0;	
	virtual void set_attr(uint32_t id, double value) = 0;
	virtual struct unit_path *get_unit_path() = 0;
	virtual float get_speed() = 0;
	virtual void update_sight(area_struct *old_area, area_struct *new_area) = 0;
	virtual bool on_player_leave_sight(uint64_t player_id) = 0;
	virtual bool on_player_enter_sight(uint64_t player_id) = 0;
	virtual bool on_monster_leave_sight(uint64_t uuid) = 0;
	virtual bool on_monster_enter_sight(uint64_t uuid) = 0;
	virtual bool on_partner_leave_sight(uint64_t player_id) = 0;
	virtual bool on_partner_enter_sight(uint64_t player_id) = 0;	
	virtual int *get_cur_sight_player() = 0;
	virtual uint64_t *get_all_sight_player() = 0;
	virtual int *get_cur_sight_monster() = 0;
	virtual uint64_t *get_all_sight_monster() = 0;
	virtual int *get_cur_sight_partner() = 0;
	virtual uint64_t *get_all_sight_partner() = 0;	
	virtual int *get_cur_sight_truck() = 0;
	virtual uint64_t *get_all_sight_truck() = 0;
	virtual void on_hp_changed(int damage) = 0;
	virtual void on_dead(unit_struct *killer) = 0;
	virtual void on_beattack(unit_struct *player, uint32_t skill_id, int32_t damage);
	virtual void on_repel(unit_struct *player) = 0;  //击退
	virtual bool is_avaliable() = 0;
	virtual uint32_t get_skill_id() = 0;
	virtual JobDefine get_job() = 0;
	virtual void do_taunt_action() = 0;  //嘲讽

	int count_rect_unit_at_pos(double angle, struct position *start_pos, std::vector<unit_struct *> *ret, uint max, double length, double width);	
	int count_rect_unit(double angle, std::vector<unit_struct *> *ret, uint max, double length, double width);
	int count_circle_unit(std::vector<unit_struct *> *ret, uint max, double radius);
	int count_fan_unit(std::vector<unit_struct *> *ret, uint max, double radius, double angle);
	
	virtual bool give_drop_item(uint32_t drop_id, uint32_t statis_id, AddItemDealWay deal_way, bool isNty = true, uint32_t mail_id = 0, std::vector<char *> *mail_args = NULL); //发放掉落奖励
	virtual void broadcast_one_attr_changed(uint32_t id, double value, bool send_team, bool include_myself);
	void broadcast_buff_state_changed();
	struct position *get_pos();
	int check_pos_distance(float pos_x, float pos_z);
	void reset_pos();
	int set_pos_with_broadcast(float pos_x, float pos_z);	
	bool is_unit_in_move();
	void set_pos(float pos_x, float pos_z);
	bool is_alive();
	bool is_player_in_sight(uint64_t player_id);	
	int add_player_to_sight(uint64_t player_id);
	int del_player_from_sight(uint64_t player_id);
	bool is_monster_in_sight(uint64_t uuid);	
	int add_monster_to_sight(uint64_t uuid);
	int del_monster_from_sight(uint64_t uuid);
	bool is_truck_in_sight(uint64_t uuid);
	int add_truck_to_sight(uint64_t uuid);
	int del_truck_from_sight(uint64_t uuid);
	bool is_partner_in_sight(uint64_t player_id);	
	int add_partner_to_sight(uint64_t player_id);
	int del_partner_from_sight(uint64_t player_id);
	bool is_unit_in_sight(uint64_t player_id);
	
	void delete_state_buff(int state);
	int get_free_buff_pos();
	void set_one_buff(buff_struct *buff, int pos);	
	void delete_one_buff(buff_struct *buff);
	void clear_all_buffs();
	buff_struct *try_cover_duplicate_buff(struct BuffTable *buff_config, uint64_t end_time, unit_struct *attack);
	void reset_unit_buff_state();
	int stop_move();
	int clear_debuff();
	int clear_control_buff();
	bool add_watched_list(uint64_t player_id);
	bool in_watched_list(uint64_t player_id);
	void clear_watched_list();
	
		//发送视野广播
	void broadcast_to_sight(uint16_t msg_id, void *msg_data, pack_func func, bool include_myself);
		//发送广播到两个人的视野
//	void broadcast_to_sight_with_other_unit(uint16_t msg_id, void *msg_data, pack_func func, bool include_myself, unit_struct *other, bool include_other);
		//发送广播到一组人的视野
	static void broadcast_to_many_sight(uint16_t msg_id, void *msg_data, pack_func func, const std::vector<unit_struct *> &other);

	static unit_struct *get_unit_by_uuid(uint64_t uuid);	
	static double *get_attr_by_uuid(uint64_t uuid);
	static struct position *get_pos_by_uuid(uint64_t uuid);

	int update_unit_position();
	PosData **pack_unit_move_path(size_t *n_data);

	uint32_t count_life_steal_effect(int32_t damage);
	uint32_t count_damage_return(int32_t damage, unit_struct *unit);
	
	static void init_pos_pool();
	static void reset_pos_pool();

	BuffInfo **pack_unit_buff(size_t *n_data);
	static void init_buff_pool();
	static void reset_buff_pool();

	static void reset_pools();
	
	bool is_in_lock_time();
	void set_lock_time(uint64_t t);
	
	scene_struct *scene;
	area_struct *area;
	uint64_t lock_time;  //硬直时间
	uint32_t buff_state; //眩晕，无敌等状态
//	uint32_t life_steal;  //吸血
//	uint32_t damage_return; //反弹
	buff_struct *m_buffs[MAX_BUFF_PER_UNIT];

protected:
	unit_struct *get_taunt_target();
	void calculate_buff_fight_attr(bool isNty);	
	buff_struct *try_cover_duplicate_item_buff(struct BuffTable *buff_config);
	buff_struct *try_cover_duplicate_skill_buff(struct BuffTable *buff_config, uint64_t end_time, unit_struct *attack);
	buff_struct *try_cover_duplicate_type3_buff(struct BuffTable *buff_config);

	bool pos_changed;   //坐标是否变化过，如果变化过，就发送给下面的监视列表
	std::list<uint64_t> watched_player_id; //位置信息的变化要通知这些人，没放在共享内存，重启后就清空了
};

#endif /* UNIT_H */
