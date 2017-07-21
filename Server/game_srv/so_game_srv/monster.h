#ifndef MONSTER_H
#define MONSTER_H

#include <stdint.h>
#include "player.h"
#include "game_config.h"
/*
struct fight_attr
{
	int32_t hp;
	int32_t attack;
	int32_t defense;
	int32_t at_gold;
	int32_t at_wood;
	int32_t at_water;
	int32_t at_fire;
	int32_t at_earth;
	int32_t df_gold;
	int32_t df_wood;
	int32_t df_water;
	int32_t df_fire;
	int32_t df_earth;
	int32_t dodge;
	int32_t hit;
	int32_t critical;
	int32_t critical_df;
	int32_t ct_dmg;
	int32_t recover;
	int32_t cure_add;
	int32_t speed;
	int32_t dissy_df;
	int32_t slowdf;
	int32_t chaos_df;
};
*/

#define MAX_PLAYER_IN_MONSTER_SIGHT 30
#define MAX_MONSTER_IN_MONSTER_SIGHT 100
#define MAX_TRUCK_IN_MONSTER_SIGHT (30)
#define MAX_PARTNER_IN_MONSTER_SIGHT (30)
#define MAX_MONSTER_SKILL (10)

struct monster_data
{
	int create_config_index;
	uint64_t player_id;
	uint32_t monster_id;
	uint32_t scene_id;  //场景ID 
	uint32_t guild_id;  //帮会ID 
	uint64_t raid_uuid; //副本唯一ID
//	struct fight_attr attr;
	double attrData[PLAYER_ATTR_FIGHT_MAX]; //战斗属性
	double buff_fight_attr[MAX_BUFF_FIGHT_ATTR]; //战斗算上buff百分比属性

//	struct position pos;
//	float pos_y;  //注意y是高度
//	float speed;  //速度
//移动路径
	struct unit_path move_path;
	float born_direct;

	uint64_t sight_player[MAX_PLAYER_IN_MONSTER_SIGHT];
	int cur_sight_player;

	uint64_t sight_monster[MAX_MONSTER_IN_MONSTER_SIGHT];
	int cur_sight_monster;	

	uint64_t sight_truck[MAX_TRUCK_IN_MONSTER_SIGHT];
	int cur_sight_truck;

	uint64_t sight_partner[MAX_PARTNER_IN_MONSTER_SIGHT];
	int cur_sight_partner;	

	int heap_index;
	uint64_t ontick_time;

	uint32_t next_skill_id;  //AI控制的下一个选择释放的技能
	uint32_t skill_id;  //即将释放的技能
	uint64_t skill_finished_time;
	double angle;     //技能的角度
	struct position skill_target_pos; //技能释放的位置，现在只有SKILL_RANGE_TYPE_TARGET_RECT类型的技能用
	struct position target_pos; //追击对象的位置，如果没有发生变化就不需要重新寻路

	uint8_t camp_id;

	uint64_t relive_time;
	uint64_t owner;  //主人
	bool stop_ai; //停止AI
};

typedef void (*ai_init)(monster_struct *, unit_struct *);
typedef void (*ai_tick)(monster_struct *);
typedef void (*ai_beattack)(monster_struct *, unit_struct *);
typedef void (*ai_raid_attack_player)(monster_struct *, player_struct *, int);
typedef void (*ai_dead)(monster_struct *, scene_struct *scene);
typedef void (*ai_hp_changed)(monster_struct *);
typedef void (*ai_alive)(monster_struct *);
typedef bool (*ai_on_player_leave_sight)(monster_struct *, player_struct *);
typedef void (*ai_owner_attack)(monster_struct *, player_struct *, unit_struct *);
typedef void (*ai_owner_beattack)(monster_struct *, player_struct *, unit_struct *);
typedef bool (*ai_check_goback)(monster_struct *);
typedef unit_struct *(*ai_choose_target)(monster_struct *);
typedef void (*ai_do_goback)(monster_struct *);
struct ai_interface
{
	ai_tick on_tick; //定时驱动
	ai_beattack on_beattack;  //受击
	ai_dead on_dead;   //死亡
	ai_beattack on_fly;	  //被击飞击倒击退
	ai_alive on_alive;   //重生，创建
	ai_on_player_leave_sight on_player_leave_sight;  //玩家离开视野
	ai_owner_attack on_owner_attack;   //主人攻击()
	ai_owner_attack on_owner_beattack;	//主人被攻击()
	ai_raid_attack_player on_raid_attack_player;  //副本中攻击玩家
	ai_hp_changed on_hp_changed;   //死亡
	ai_init on_monster_ai_init;  //初始化
	ai_check_goback on_monster_ai_check_goback; //检查是否需要走回去
	ai_choose_target monster_ai_choose_target;  //主动攻击的时候寻找攻击目标
	ai_do_goback on_monster_ai_do_goback;   //执行走回去
};

enum AIStateType
{
	AI_ATTACK_STATE = 0,    //攻击模式
	AI_PURSUE_STATE,    //追击模式
	AI_GO_BACK_STATE,   //返回模式
	AI_PATROL_STATE,   //巡逻模式
//	AI_DIRECT_PATROL_STATE,    //直巡模式
	AI_PRE_GO_BACK_STATE,   //预返回模式
	AI_DEAD_STATE,      //死亡
	AI_WAIT_STATE,     //等待状态
	AI_STAND_STATE,    //傻站状态
};
enum AIType
{
	AI_TYPE_STAND = 0,  //固定不巡逻
	AI_TYPE_NORMAL = 1,  //随机巡逻
	AI_TYPE_CIRCLE = 2,      //固定点巡逻	
	AI_TYPE_WOOD = 3,  	//木桩类型
	AI_TYPE_ESCORT = 14,      //护送
};

#define MAX_CIRCLE_POS_NUM 10 

class monster_struct: public unit_struct
{
public:
	virtual ~monster_struct();
	UNIT_TYPE get_unit_type();	
	bool is_avaliable();
	uint32_t get_skill_id();
	uint64_t get_uuid();
	double *get_all_attr();
	double get_attr(uint32_t id);
	double *get_all_buff_fight_attr();
	double get_buff_fight_attr(uint32_t id);
	void clear_cur_skill();

	bool can_beattack();
	bool is_in_safe_region();
	int get_camp_id();
	void set_camp_id(int id);	
	Team *get_team();
//	int get_pk_type();

	virtual int add_skill_cd(uint32_t index, uint64_t now);
	virtual bool is_skill_in_cd(uint32_t index, uint64_t now);

//	int enter_raid(raid_struct *raid);
//	int leave_raid();

	void send_patrol_move();	
	void do_taunt_action();
	void go_back();
	
	void set_attr(uint32_t id, double value);
	struct unit_path *get_unit_path();
	float get_speed();
	int *get_cur_sight_player();
	uint64_t *get_all_sight_player();
	int *get_cur_sight_monster();
	uint64_t *get_all_sight_monster();
	virtual int *get_cur_sight_truck();
	virtual uint64_t *get_all_sight_truck();
	int *get_cur_sight_partner();
	uint64_t *get_all_sight_partner();	
	
	JobDefine get_job();
	
	static void add_ai_interface(int ai_type, struct ai_interface *ai);
	void calculate_attribute(void);
	void set_ai_interface(int ai_type);
	virtual void init_monster();
	virtual void on_tick();
	void on_dead(unit_struct *killer);
	void on_hp_changed(int damage);	
	virtual void on_relive();
	void on_beattack(unit_struct *player, uint32_t skill_id, int32_t damage);
	virtual void reset_timer(uint64_t time);
	virtual void set_timer(uint64_t time);
	virtual void on_go_back();
	virtual void on_pursue();
	virtual void clear_monster_timer();
	
	void on_repel(unit_struct *player);
	void pack_sight_monster_info(SightMonsterInfo *info);
	int broadcast_monster_delete(bool send_msg);
	int broadcast_monster_create(uint32_t effectid);
	int broadcast_monster_move();
//	int add_player_to_sight(uint64_t player_id);
//	int del_player_from_sight(uint64_t player_id);

	void clear_monster_sight();
	void update_sight(area_struct *old_area, area_struct *new_area);
	int prepare_add_monster_to_sight(monster_struct *monster);
	int prepare_add_player_to_sight(player_struct *player);
	int prepare_add_truck_to_sight(cash_truck_struct *truck);
	int prepare_add_partner_to_sight(partner_struct *partner);

	uint64_t count_rand_patrol_time();  //计算一个巡逻的定时器时间

		//计算技能命中的对象
	int count_skill_hit_unit(std::vector<unit_struct *> *ret, struct SkillTable *config, unit_struct *target);

	void cast_immediate_skill_to_player(uint64_t skill_id, unit_struct *player);

	inline float get_born_pos_x()
	{
		assert(create_config);
//		assert(ai_type != AI_TYPE_CIRCLE);
		return create_config->PointPosX;
	}
	inline float get_born_pos_z()
	{
		assert(create_config);
//		assert(ai_type != AI_TYPE_CIRCLE);
		return create_config->PointPosZ;
	}

	bool on_player_leave_sight(uint64_t player_id);
	bool on_player_enter_sight(uint64_t player_id);
	bool on_monster_leave_sight(uint64_t uuid);
	bool on_monster_enter_sight(uint64_t uuid);
	bool on_partner_leave_sight(uint64_t player_id);
	bool on_partner_enter_sight(uint64_t player_id);

	int del_monster_from_sight_both(monster_struct *monster);
	int add_monster_to_sight_both(monster_struct *monster);
	int del_truck_from_sight_both(cash_truck_struct *truck);
	int add_truck_to_sight_both(cash_truck_struct *truck);
	int del_partner_from_sight_both(partner_struct *partner);
	int add_partner_to_sight_both(partner_struct *partner);		

	void add_sight_space_player_to_sight(sight_space_struct *sight_space, uint16_t *add_player_id_index, uint64_t *add_player);
	void add_sight_space_monster_to_sight(sight_space_struct *sight_space);
	void add_sight_space_partner_to_sight(sight_space_struct *sight_space);	
	void add_sight_space_truck_to_sight(sight_space_struct *sight_space);
	
//	int count_rect_unit(std::vector<unit_struct *> *ret, uint max, double length, double width);	
	
//移动路径
//	struct unit_path move_path;	
	bool can_see_player(player_struct *player);
	void update_monster_pos_and_sight();
	bool try_active_attack();	
	struct monster_data *data;

	struct ai_interface *ai;
	uint16_t ai_type;
	uint16_t ai_state;
	unit_struct *target;  //攻击目标，攻击目标必然也在玩家的视野之中，所以玩家下线的时候需要遍历视野，清理所有对应怪物的目标字段
	union
	{
//		struct
//		{
//			struct position born_pos;  //出生点
//		} normal_ai;  //随机移动怪物AI
		struct
		{
			struct position ret_pos;  //被吸引进战斗的地点，只会返回该地点
			uint8_t cur_pos_index;			
//			uint8_t n_circle;
//			struct position circle[MAX_CIRCLE_POS_NUM];
		} circle_ai; //转圈怪物AI以及单线不返回巡逻怪
		struct
		{
			uint8_t cur_pos_index;
			uint64_t out_fight_time;  //脱战时间
			uint64_t owner_player_id;  //所属的玩家
			uint32_t escort_id; //护送ID
			EscortTask *config;
			struct position ret_pos;  //被吸引进战斗的地点，只会返回该地点
		} escort_ai; //护送怪物AI
		struct
		{
			uint8_t type;   //类型
			bool can_use_leiminggu; //玩家能否使用雷鸣鼓
			uint32_t call_skill_id;  //召唤技能ID
			uint8_t next_leiminggu_hp_percent; //下次触发雷鸣鼓的血量百分比
			uint8_t state;   //状态 0:初始，1：雷鸣鼓，还未召唤，2：雷鸣鼓，已经召唤
			uint64_t leminggu_time; //雷鸣鼓技能释放时间
			uint64_t meihuo_time;
			uint32_t leiminggu_collect_id;			//采集物唯一ID，记录用来删除
		} leixinye_ai; //雷薪野AI
		struct
		{
			uint64_t trigger_time;//AI定时时间，用来控制技能伤害次数
			uint32_t hurt_flag;	  //是否是第一次出伤害，如果是，有延时时间就用延时时间
		} feijian_ai; //飞箭陷阱AI
		struct
		{
			uint8_t state; //状态，0和1
			uint64_t call_skill_time;  //召唤技能CD
			uint8_t call_monster_num; //已经召唤的怪物数量，最多20
		} muxue_jiangshi_ai;
		struct
		{
			uint64_t boss_uuid;  //要加血的boss的uuid
		} add_boss_hp_ai;
		struct
		{
			uint8_t state; //1: 参加战斗5秒之后BOSS会开始释放技能并且冒泡说话，秋戈对玩家释放技能2和技能3，并且继续使用技能1加血和冒泡说话。
			uint64_t state1_time; //进入state1的时间
		} qiuge_ai;
		struct
		{
			uint8_t state;   //0: 初始状态   1：变身中  2：变身完成   2: 变身且血量低于20%
		} yaodi_ai;
		struct
		{
			uint8_t state;   //1: 血量低于50%的时候，属性提升  2: 血量低于10%的时候，进入虚弱状态
		} type21_ai;
	} ai_data;

	struct MonsterTable *config;
	struct BaseAITable *ai_config;
	struct SceneCreateMonsterTable *create_config;  

	uint32_t drop_id;
	sight_space_struct *sight_space;
	uint64_t m_liveTime;
	bool mark_delete;
	uint64_t skill_cd[MAX_MONSTER_SKILL];
private:
	void del_sight_player_in_area(int n_del, area_struct **del_area, int *delete_player_id_index, uint64_t *delete_player_id);
	void del_sight_monster_in_area(int n_del, area_struct **del_area);
	void del_sight_truck_in_area(int n_del, area_struct **del_area);
	void del_sight_partner_in_area(int n_del, area_struct **del_area);		
	void add_area_player_to_sight(area_struct *area, uint16_t *add_player_id_index, uint64_t *add_player);
	void add_area_monster_to_sight(area_struct *area);
	void add_area_truck_to_sight(area_struct *area);
	void add_area_partner_to_sight(area_struct *area);			
		
//	void add_to_area_player_sight(area_struct *area, uint64_t *ppp, uint16_t *index);
//	int count_circle_unit(std::vector<unit_struct *> *ret, uint max, double radius);
//	int count_fan_unit(std::vector<unit_struct *> *ret, uint max, double radius, double angle);
//	int count_rect_unit_at_pos(struct position *pos, std::vector<unit_struct *> *ret, uint max, double length, double width);	
};

#endif /* MONSTER_H */
