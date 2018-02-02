#ifndef __PARTNER_H__
#define __PARTNER_H__

#include "unit.h"
#include "player.h"
#include "sight_space.h"

class partner_struct;

//typedef void (*partner_ai_init)(partner_struct *);
typedef int (*partner_ai_tick)(partner_struct *);
//typedef void (*partner_ai_beattack)(partner_struct *, unit_struct *);
//typedef void (*partner_ai_raid_attack_player)(partner_struct *, player_struct *, int);
//typedef void (*partner_ai_dead)(partner_struct *, scene_struct *scene);
typedef void (*partner_ai_hp_changed)(partner_struct *);
typedef unit_struct *(*partner_ai_choose_target)(partner_struct *);
//typedef void (*partner_ai_alive)(partner_struct *);
//typedef bool (*partner_ai_on_player_leave_sight)(partner_struct *, player_struct *);
//typedef void (*partner_ai_owner_attack)(partner_struct *, player_struct *, unit_struct *);
//typedef void (*partner_ai_owner_beattack)(partner_struct *, player_struct *, unit_struct *);
struct partner_ai_interface
{
	partner_ai_tick on_tick; //定时驱动
	partner_ai_choose_target choose_target;  //选择攻击目标
//	partner_ai_beattack on_beattack;  //受击
//	partner_ai_dead on_dead;   //死亡
//	partner_ai_beattack on_fly;	  //被击飞击倒击退
//	partner_ai_alive on_alive;   //重生，创建
//	partner_ai_on_player_leave_sight on_player_leave_sight;  //玩家离开视野
//	partner_ai_owner_attack on_owner_attack;   //主人攻击()
//	partner_ai_owner_attack on_owner_beattack;	//主人被攻击()
//	partner_ai_raid_attack_player on_raid_attack_player;  //副本中攻击玩家
	partner_ai_hp_changed on_hp_changed;   //死亡
//	partner_ai_init on_partner_ai_init;  //初始化
};

struct PartnerSkill
{
	uint32_t skill_id;
	uint32_t exp;
	uint32_t lv;
	uint64_t cd;
	bool lock;
};

#define MAX_PLAYER_IN_PARTNER_SIGHT 30
#define MAX_MONSTER_IN_PARTNER_SIGHT 100
#define MAX_TRUCK_IN_PARTNER_SIGHT (30)
#define MAX_PARTNER_IN_PARTNER_SIGHT (30)
#define  MAX_PARTNER_BASE_ATTR (5)
#define  MAX_PARTNER_DETAIL_ATTR (26)
#define  MAX_PARTNER_GOD (10)


struct partner_attr_data
{
	//uint32_t base_attr_id[MAX_PARTNER_BASE_ATTR];
	uint32_t base_attr_vaual[MAX_PARTNER_BASE_ATTR];
	uint32_t base_attr_up[MAX_PARTNER_BASE_ATTR];

	uint32_t detail_attr_id[MAX_PARTNER_DETAIL_ATTR];
	uint32_t detail_attr_vaual[MAX_PARTNER_DETAIL_ATTR];
	uint32_t n_detail_attr;
	uint32_t type;
	double power_refresh;

	PartnerSkill skill_list[MAX_PARTNER_SKILL_NUM];
};

struct partner_cur_fabao
{
	uint32_t fabao_id; //法宝道具id
	AttrInfo main_attr; //当前法宝主属性
	AttrInfo minor_attr[MAX_HUOBAN_FABAO_MINOR_ATTR_NUM]; //当前法宝副属性
};
struct partner_data
{
	uint64_t uuid; //唯一ID
	uint32_t partner_id; //配置表ID
	uint64_t owner_id; //主人ID
	double attrData[MAX_PARTNER_ATTR]; //战斗属性 需要战斗力属性
	double buff_fight_attr[MAX_BUFF_FIGHT_ATTR]; //战斗算上buff百分比属性

	char name[MAX_PLAYER_NAME_LEN + 1];    //名字
	bool     partner_rename_free;   //首次改名

	partner_attr_data attr_cur;   //当前属性
	partner_attr_data attr_flash;  //洗髓属性

	uint32_t god_id[MAX_PARTNER_GOD];
	uint32_t god_level[MAX_PARTNER_GOD];
	uint32_t n_god;
	uint32_t strong_num; //强化次数 

	bool stop_ai; //停止AI	

		//定时器
	int heap_index;
	uint64_t ontick_time;

//	uint32_t scene_id;  //场景ID 	
	struct unit_path move_path;	
	uint64_t sight_player[MAX_PLAYER_IN_PARTNER_SIGHT];
	int cur_sight_player;

	uint64_t sight_monster[MAX_MONSTER_IN_PARTNER_SIGHT];
	int cur_sight_monster;	

	uint64_t sight_truck[MAX_TRUCK_IN_PARTNER_SIGHT];
	int cur_sight_truck;

	uint64_t sight_partner[MAX_PARTNER_IN_PARTNER_SIGHT];
	int cur_sight_partner;	

	uint32_t bind; //绑定状态
	uint32_t relive_time; //复活时间

	uint32_t skill_id;  //即将释放的技能
	double angle;     //技能的角度
	struct position skill_target_pos; //技能释放的位置，现在只有SKILL_RANGE_TYPE_TARGET_RECT类型的技能用	

	struct cast_skill_data cur_skill;  //正在释放的技能	
	struct partner_cur_fabao cur_fabao; //伙伴当前佩戴的法宝
};

//战斗中的对象
typedef struct _partner_attack_unit__
{
	uint64_t uuid;
	uint64_t time;
} partner_attack_unit;
#define MAX_PARTNER_ATTACK_UNIT 20
extern const uint32_t base_attr_id[MAX_PARTNER_BASE_ATTR];
class partner_struct: public unit_struct
{
public:
	partner_struct(void);
	virtual ~partner_struct(void);
	virtual bool can_beattack();
	virtual bool is_in_safe_region();
	virtual int get_camp_id();         //红蓝阵营关系, 战斗关系优先级最高, 一方为0的时候被忽略
	virtual void set_camp_id(int id);
	virtual Team *get_team();
	virtual void clear_cur_skill(); //嘲讽的时候打断当前技能
	virtual UNIT_TYPE get_unit_type();
	virtual uint64_t get_uuid();
	virtual double *get_all_attr();
	virtual double *get_all_buff_fight_attr();	
	virtual double get_attr(uint32_t id);
	virtual double get_buff_fight_attr(uint32_t id);	
	virtual void set_attr(uint32_t id, double value);
	virtual struct unit_path *get_unit_path();
	virtual float get_speed();
	virtual void update_sight(area_struct *old_area, area_struct *new_area);
	virtual bool on_truck_leave_sight(uint64_t player_id);		
	virtual bool on_truck_enter_sight(uint64_t player_id);		
	virtual bool on_player_leave_sight(uint64_t player_id);
	virtual bool on_player_enter_sight(uint64_t player_id);
	virtual bool on_monster_leave_sight(uint64_t uuid);
	virtual bool on_monster_enter_sight(uint64_t uuid);
	virtual bool on_partner_leave_sight(uint64_t uuid);
	virtual bool on_partner_enter_sight(uint64_t uuid);		
	virtual int *get_cur_sight_player();
	virtual uint64_t *get_all_sight_player();
	virtual int *get_cur_sight_monster();
	virtual uint64_t *get_all_sight_monster();
	virtual int *get_cur_sight_partner();
	virtual uint64_t *get_all_sight_partner();		
	virtual int *get_cur_sight_truck();
	virtual uint64_t *get_all_sight_truck();
	virtual void on_hp_changed(int damage);
	virtual void on_dead(unit_struct *killer);
	virtual void on_repel(unit_struct *player);  //击退
	virtual bool is_avaliable();
	virtual uint32_t get_skill_id();
	virtual JobDefine get_job();
	virtual void do_taunt_action();  //嘲讽
	virtual void on_tick();
	virtual player_struct *get_owner();
	
	void reset_timer(uint64_t time);
	void set_timer(uint64_t time);
	void update_partner_pos_and_sight();
	void on_relive();

	
	void set_ai_interface(int ai_type);	
	static void add_ai_interface(int ai_type, struct partner_ai_interface *ai);
	
	int prepare_add_monster_to_sight(monster_struct *monster);
	int prepare_add_player_to_sight(player_struct *player);
	int prepare_add_truck_to_sight(cash_truck_struct *truck);
	int prepare_add_partner_to_sight(partner_struct *partner);

	void del_sight_player_in_area(int n_del, area_struct **del_area, int *delete_player_id_index, uint64_t *delete_player_id);
	void del_sight_monster_in_area(int n_del, area_struct **del_area);
	void del_sight_truck_in_area(int n_del, area_struct **del_area);
	void del_sight_partner_in_area(int n_del, area_struct **del_area);		
	void add_area_player_to_sight(area_struct *area, uint16_t *add_player_id_index, uint64_t *add_player);
	void add_area_monster_to_sight(area_struct *area);
	void add_area_truck_to_sight(area_struct *area);
	void add_area_partner_to_sight(area_struct *area);			

	int add_partner_to_sight_both(partner_struct *partner);
	int del_partner_from_sight_both(partner_struct *partner);

	void add_sight_space_player_to_sight(sight_space_struct *sight_space, uint16_t *add_player_id_index, uint64_t *add_player);	
	void add_sight_space_monster_to_sight(sight_space_struct *sight_space);
	
	int broadcast_partner_delete(bool send_msg);
	int broadcast_partner_create();
	void pack_sight_partner_info(SightPartnerInfo *info);
	void broadcast_to_sight_and_owner(uint16_t msg_id, void *msg_data, pack_func func, bool include_owner);

	int init_partner(uint32_t partner_id, player_struct *owner); //初始化简单的数据
	int init_create_data(void); //新创建的伙伴要初始化随机的数据
	int init_end(bool isNty); //初始化最后一步，其他通过配置表和计算得出的数据
//	void clear(void);
	void relesh_attr();

	void on_owner_attack(uint64_t uuid);
	void on_owner_beattack(uint64_t uuid);
	virtual void on_beattack(unit_struct *unit, uint32_t skill_id, int32_t damage);	

	void calc_target_pos(struct position *pos);  //计算伙伴跟随的位置，右后方45度5米距离

	//属性
	void calculate_attribute(bool isNty = false);
	void calculate_attribute(double *attrData, partner_attr_data &attr_cur);
	void notify_attr(AttrMap& attr_list, bool broadcast = false, bool include_owner = true);
	void notify_one_attr_changed(uint32_t attr_id, double attr_val);
	
	//经验
	int add_exp(uint32_t val, uint32_t statis_id, uint32_t owner_lv, bool isNty = true);
	int deal_level_up(uint32_t level_old, uint32_t level_new);
	int get_total_exp(void);
	uint32_t get_level();
		//返回0表示没有继续执行跟随主人的AI，返回1表示不继续执行跟随主人
	uint32_t attack_target(uint32_t skill_id, int skill_index, unit_struct *target);
	uint32_t choose_skill(int *index);
	int lock_skill(uint32_t skill, bool lock);
	bool try_friend_skill(uint32_t skill_id, int skill_index);
	void do_normal_attack();

//	void cast_skill_to_friend(struct SkillTable *config);
//	void try_attack_friend(struct SkillTable *config);
	
	void mark_bind(void);

	struct PartnerTable *config;
	struct partner_data *data;
	player_struct *m_owner;

	struct partner_ai_interface *ai;
	uint16_t ai_type;
	uint16_t ai_state;
	partner_attack_unit attack_owner[MAX_PARTNER_ATTACK_UNIT]; //正在攻击主人
	partner_attack_unit attack_partner[MAX_PARTNER_ATTACK_UNIT];  //正在攻击伙伴自己
	partner_attack_unit owner_attack[MAX_PARTNER_ATTACK_UNIT];  //主人正在攻击的目标
	sight_space_struct *partner_sight_space;
	
	unit_struct *m_target;  //攻击目标
	union
	{
		struct
		{
		} type1_ai;
	} ai_data;

private:
	int add_skill_cd(int index, uint64_t now);
	bool is_skill_in_cd(uint32_t index, uint64_t now);
	int broadcast_partner_move();	
	void clear_partner_sight();
	void send_patrol_move();
	void cast_skill_to_target(uint64_t skill_id, unit_struct *target);	
//	void cast_immediate_skill_to_target(uint64_t skill_id, int skill_index, unit_struct *target);

	virtual double get_skill_angle();
	virtual struct position *get_skill_target_pos();
//	int count_skill_hit_unit(std::vector<unit_struct *> *ret, struct SkillTable *config, bool bfriend);
//	void hit_notify_to_player(uint64_t skill_id, unit_struct *target);	
//	void hit_notify_to_many_player(uint64_t skill_id, std::vector<unit_struct *> *target);
//	void hit_notify_to_many_friend(uint64_t skill_id, std::vector<unit_struct *> *target);
//	void try_attack(struct SkillTable *config);
//	void try_attack_target(struct SkillTable *config);
	int get_skill_level(uint32_t skill_id);
	int get_skill_level_byindex(int skill_index);	
};

#endif /* __PARTNER_H__ */
