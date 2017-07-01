#ifndef _PLAYER_H__
#define _PLAYER_H__
#include <stdint.h>
#include <stdio.h>
#include <set>
#include <list>
#include <string.h>
#include <vector>
#include <string>
#include <queue>
#include <map>
#include "conn_node_gamesrv.h"
#include "unit_path.h"
#include "attr_id.h"
#include "unit.h"
#include "../proto/move.pb-c.h"
#include "comm_define.h"
#include "my_skill.h"
#include "game_helper.h"

#define UNUSED(x) (void)(x)

class scene_struct;
struct area_struct;
class monster_struct;
class sight_space_struct;
class Team;
class raid_struct;
struct _PlayerDBInfo;
struct ItemsConfigTable;
class partner_struct;
class cash_truck_struct;

typedef std::map<uint64_t, partner_struct *> PartnerMap;

enum
{
	SIGHT_PRIORITY_NORMAL = 100,
};

#define MAX_PLAYER_IN_PLAYER_SIGHT 100
#define MAX_TRUCK_IN_PLAYER_SIGHT 30
#define MAX_MONSTER_IN_PLAYER_SIGHT 200
#define MAX_COLLECT_IN_PLAYER_SIGHT 50
#define MAX_PARTNER_IN_PLAYER_SIGHT 50

static const int AWARD_QUESTION_ACTIVE_ID = 330100021;
static const int COMMON_QUESTION_ACTIVE_ID = 330100020;

struct AttrInfo
{
	uint32_t id;
	double val;
};

struct cast_skill_data
{
	float direct_x;
	float direct_z;	
	uint32_t skill_id;
	uint64_t start_time;
};

struct ItemBaguapaiInfo
{
	uint32_t star;
	double main_attr_val;
	AttrInfo minor_attrs[MAX_BAGUAPAI_MINOR_ATTR_NUM];
};

struct bag_grid_data
{
	uint32_t id;
	uint32_t num;
	uint32_t used_count;
	uint32_t expire_time;
	ItemBaguapaiInfo baguapai;
};

struct HeadIconInfo
{
	uint32_t id;
	uint32_t status; //状态
};

struct TaskCountInfo
{
	uint32_t id;
	uint32_t num;
};

struct TaskInfo
{
	uint32_t id;
	uint32_t status;
	uint32_t accept_ts; //接任务时间
	uint32_t accu_time; //累计时间
	TaskCountInfo progress[MAX_TASK_TARGET_NUM];
};
typedef std::set<uint32_t> TaskSet;

struct SingInfo
{
	uint32_t type;
	uint32_t time;
	uint32_t obj_id;
	bool broad;
	bool include_myself;
	uint64_t start_ts;
};

//struct player_raid_data
//{
//	uint32_t scene_id;   //从副本返回的场景
//	uint32_t pos_x;   //从副本返回的坐标
//	uint32_t pos_z;
//};

struct EquipEnchantInfo
{
	AttrInfo cur_attr;
	AttrInfo rand_attr[MAX_EQUIP_ENCHANT_RAND_NUM];
};

struct EquipInfo
{
	uint32_t stair; //阶数
	uint32_t star_lv; //星数
	uint32_t star_exp; //升星经验
	EquipEnchantInfo enchant[MAX_EQUIP_ENCHANT_NUM]; //附魔
	int32_t inlay[MAX_EQUIP_INLAY_NUM]; //镶嵌
};

struct FashionInfo 
{
	uint32_t id;
	uint32_t color;
	uint32_t colordown;
	time_t timeout;
	bool isNew;
};

struct HorseInfo 
{
	uint32_t id;
	time_t timeout;
	bool isNew;
};
const int MAX_HORSE_ATTR_NUM = 4;
const int MAX_HORSE_SOUL = 8;
struct HorseCommonInfo
{
	uint32_t step;
	uint32_t soul;
	uint32_t soul_exp[MAX_HORSE_SOUL + 1];
	uint32_t cur_soul;
	uint32_t attr[MAX_HORSE_ATTR_NUM + 1];
	uint32_t attr_exp[MAX_HORSE_ATTR_NUM + 1];
	uint32_t n_attr;
	bool soul_full;
	uint64_t power;
	uint32_t total[PLAYER_ATTR_FIGHT_MAX];
	int fly;
};

struct GoodsInfo
{
	uint32_t goods_id;
	uint32_t bought_num;
};

struct YuqidaoMaiInfo
{
	uint32_t mai_id;
	uint32_t acupoint_id;
	uint32_t fill_lv;
};

struct YuqidaoBreakInfo
{
	uint32_t id;
	uint32_t cur_val[MAX_YUQIDAO_BREAK_ATTR_NUM];
	uint32_t new_val[MAX_YUQIDAO_BREAK_ATTR_NUM];
	uint32_t new_addn[MAX_YUQIDAO_BREAK_ATTR_NUM];
	uint32_t count; //保底计数
};

#define MAX_ONEDAY_PVP_BOX 3
#define PVP_TYPE_DEFINE_3 0
#define PVP_TYPE_DEFINE_5 1
static const int MAX_SHANGJIN_NUM = 3;
static const int MAX_SHANGJIN_AWARD_NUM = 3;

//pvp副本状态
enum pvp_match_state
{
	pvp_match_state_out = 0,  //没有参加
	pvp_match_state_waiting_3 = 10, //3v3等待中
	pvp_match_state_waiting_5, //5v5等待中
	pvp_match_state_not_ready_3, //3v3等待ready
	pvp_match_state_not_ready_5, //5v5等待ready
	pvp_match_state_ready_3, //3v3 ready
	pvp_match_state_ready_5, //5v5ready
	pvp_match_state_start_3, //3v3 开启
	pvp_match_state_start_5, //5v5 开启
};

struct pvp_raid_data
{
	uint32_t pvp_raid_cd;
	pvp_match_state state;
	uint64_t matched_index;

	uint16_t oneday_win_num_3;  //当天胜利次数	
	uint8_t level_3;  //段位
	uint8_t star_3;   //星级
	uint32_t cur_level_id_3;  //当前段位ID
	uint32_t max_level_id_3; //历史最高段位ID
	uint16_t max_score_3; //历史最高积分
	uint16_t score_3;  //积分
	uint32_t avaliable_reward_level_3;  //可以领取的段位奖励
	uint32_t avaliable_box_3[MAX_ONEDAY_PVP_BOX];  //今日剩余还没领取的宝箱下标，从0开始

	uint16_t oneday_win_num_5;  //当天胜利次数	
	uint8_t level_5;  //段位
	uint8_t star_5;   //星级
	uint32_t cur_level_id_5; //当前段位ID
	uint32_t max_level_id_5; //历史最高段位ID
	uint16_t max_score_5; //历史最高积分
	uint16_t score_5;  //积分
	uint32_t avaliable_reward_level_5;  //可以领取的段位奖励
	uint32_t avaliable_box_5[MAX_ONEDAY_PVP_BOX];  //今日剩余还没领取的宝箱下标，从0开始	
};

struct BaguapaiCardInfo
{
	uint32_t id; //八卦牌ID
	uint32_t star;
	double main_attr_val;
	double main_attr_val_new;
	AttrInfo minor_attrs[MAX_BAGUAPAI_MINOR_ATTR_NUM];
	AttrInfo minor_attrs_new[MAX_BAGUAPAI_MINOR_ATTR_NUM];
};

struct BaguapaiDressInfo
{
	BaguapaiCardInfo card_list[MAX_BAGUAPAI_DRESS_NUM];
};

struct GuoYu
{
	int32_t guoyu_level;
	int32_t cur_exp;
	int32_t cur_task; //0 没任务
	uint64_t task_timeout; //0 无cd  
	uint64_t critical_cd; // 紧急cd 0没开
	uint64_t critical_next_refresh; // 紧急cd下次刷新时间
	int32_t critical_num; // 紧急剩余次数
	int32_t guoyu_num; //国御剩余收益次数
	int32_t type; //当前难度
	int32_t map;
	uint32_t random_map;
	bool award; //高级奖励
};
struct ChengJie
{
	int32_t level;
	int32_t cur_exp;
	uint32_t cur_task; //0 没任务
	//uint64_t task_timeout; //0 无cd  
	int chengjie_num; //剩余次数
	uint64_t target;
	uint64_t rest; //下次能被悬赏cd
	bool first_hit;
};
struct ShangJinTask 
{
	uint32_t id;
	uint32_t quality;
	AttrInfo award[MAX_SHANGJIN_AWARD_NUM];
	uint32_t n_award;
	uint32_t reduce;
};
struct ShangJin
{
	int32_t level;
	int32_t cur_exp;
	ShangJinTask task[MAX_SHANGJIN_NUM];
	uint32_t cur_task; //
	int shangjin_num; //剩余次数
	bool accept;
	uint32_t free; //免费刷新次数
};

struct ZhenYing 
{
	uint32_t level;
	uint32_t exp;
	uint32_t task;
	uint32_t task_type;
	uint32_t task_num;
	uint32_t step;
	uint32_t exp_day;
	uint32_t kill_week; //战场一周杀人
	uint32_t free;
	uint32_t score_week;
	uint32_t last_week;//上次更新时间
	uint32_t week; //周任务
	uint32_t change_cd; //转换阵营CD 

	uint32_t kill;
	uint32_t death;
	uint32_t help;
	uint32_t score;
	uint32_t mine; //挖宝次数
};

static const int MAX_QUESTION_ANSWER = 4;
struct CommonAnswer 
{
	uint32_t question; //题目
	uint32_t contin; //连续答对
	uint32_t right; //答对数
	uint32_t money; //
	uint32_t exp; //
	uint32_t tip; //剩余提示次数
	uint32_t help; //剩余求助次数
	bool btip; //true 已经提示过
	bool bhelp; //true 已经求助过 
	uint32_t number; //第几题
	uint32_t answer[MAX_QUESTION_ANSWER];
	uint32_t answer_tip[MAX_QUESTION_ANSWER];

	uint64_t next_open; //下次接任务时间
};

struct NpcAnswer
{
	uint32_t trun; //第几轮 从1开始
	uint32_t npc; //当前轮的答题NPC
	uint32_t right; //答对数
	uint32_t contin; //连续答对
	uint32_t money; //
	uint32_t exp; //
	uint32_t timer; //总耗时
	uint32_t question; //题目
	uint32_t number; //第几题
	bool bOpenWin;   //打开了答题界面
	uint64_t begin_time; //

	uint64_t next_open; //下次接任务时间
};

// public enum MapAreaType
// {
// 	Area_Normal=10,//普通区域
// 	Area_Peace=11,//和平区域
// 	Area_Camp=12,//阵营区域
// 	Area_Shade = 13,//阴区域
// 	Area_Positive = 14,//阳区域
// 	Area_Positive = 15,//悬崖
// }

struct DailyActivityInfo
{
	uint32_t act_id;
	uint32_t count;
};
struct ChivalryActivityInfo
{
	uint32_t act_id;
	uint32_t val;
};

struct FightingCapacity
{
	uint32_t level;
	uint32_t equip;
	uint32_t horse;
	uint32_t yuqidao;
	uint32_t bagua;
	uint32_t guild_skill;
	uint32_t partner;
	uint32_t fashion;
	uint32_t get_total(void);
};

static const int MAX_LIVE_SKILL_NUM = 2;
struct LiveSkill 
{
	uint32_t exp[MAX_LIVE_SKILL_NUM + 1];
	uint32_t level[MAX_LIVE_SKILL_NUM + 1];
	uint32_t broken[MAX_LIVE_SKILL_NUM + 1];
	uint32_t book[MAX_LIVE_SKILL_NUM + 1];
};

struct XunBaoData 
{
	uint32_t door_map;   //场景ID
	float door_x;     //传送门坐标
	float door_z;
	float door_y;
	uint64_t cd;           //npc消失的时间
	uint32_t door_id;     //传送点ID
	bool send_next;
};

struct CashTruckData
{
	uint64_t truck_id; //镖车
	bool on_truck; //false 不在镖车上 true在
	uint32_t active_id;
	uint32_t num_coin; //粮草押镖次数
	uint32_t num_gold; //财宝押镖次数
	uint32_t jiefei;// 劫匪出现的次数 
	position pos;
	uint32_t hp;
	uint32_t scene_id;
};

//玩家离开副本后所到的位置
struct LeaveRaidPosition
{
	double direct; //退出副本朝向
	double ExitPointX;  //退出副本横坐标 
	double ExitPointY;  //退出副本高度坐标
	double ExitPointZ;  //退出副本的纵坐标
	uint32_t scene_id; //退出副本所进入的场景ID
};

struct EscortInfo
{
	uint32_t escort_id; //护送ID
	uint32_t start_time; //开始时间
	uint32_t too_far_time; //距离过远开始时间
	uint32_t summon_monster_waves[MAX_ESCORT_MONSTER_WAVE]; //召唤的波数
	uint64_t summon_monster_uuids[MAX_ESCORT_MONSTER_NUM]; //召唤的怪物ID
	uint32_t summon_num;
	bool     mark_delete;
};

struct player_data
{
	uint64_t player_id;
	PlayerStatus status;

	char name[MAX_PLAYER_NAME_LEN + 1];    //名字
	uint32_t create_time; //创角时间
//	uint32_t headicon;    //头像ID
	uint32_t scene_id;  //当前场景ID
	uint64_t player_raid_uuid; //副本唯一ID, 恢复模式的话需要用这个来恢复
	uint32_t last_scene_id; //上一个野外场景
//	bool player_is_in_loading; //是否在loading过程中，恢复模式需要用这个来恢复
//	uint16_t region_id;
//	struct position pos;
	float pos_y;  //注意y是高度
//	float speed;  //速度
	double attrData[PLAYER_ATTR_MAX]; //属性
	double buff_fight_attr[MAX_BUFF_FIGHT_ATTR]; //战斗算上buff百分比属性
	
//视野相关
	uint64_t sight_player[MAX_PLAYER_IN_PLAYER_SIGHT];
	uint64_t sight_monster[MAX_MONSTER_IN_PLAYER_SIGHT];
	uint64_t sight_truck[MAX_TRUCK_IN_PLAYER_SIGHT];
	uint64_t sight_partner[MAX_PARTNER_IN_PLAYER_SIGHT];	
	int cur_sight_player;
	int cur_sight_monster;	
	int cur_sight_truck;
	int cur_sight_partner;	

//移动路径
	struct unit_path move_path;

	SingInfo sing_info;
	bool login_notify;  //false表示第一次登陆进场景，true表示场景切换
	FightingCapacity fc_data;

	struct cast_skill_data cur_skill;  //正在释放的技能

	bag_grid_data bag[MAX_BAG_GRID_NUM]; //背包
	uint32_t bag_grid_num; //背包开启格子数
	uint32_t bag_unlock_num; //背包解锁格子数

	//头像
	HeadIconInfo head_icon_list[MAX_HEAD_ICON_NUM]; //头像列表

	//任务
	TaskInfo task_list[MAX_TASK_ACCEPTED_NUM];
	uint32_t task_finish[MAX_TASK_NUM];
	uint32_t task_chapter_reward;

	uint64_t teamid;
	uint64_t next_time_refresh_oneday_job;

//	struct player_raid_data raid_data;
	uint32_t raid_reward_id[MAX_RAID_NUM];
	uint32_t raid_reward_num[MAX_RAID_NUM];	

	//装备
	EquipInfo equip_list[MAX_EQUIP_NUM];

	//时装
	uint32_t color[MAX_COLOR_NUM];
	int color_is_new[MAX_COLOR_NUM];
	uint32_t n_color;
	uint32_t weapon_color[MAX_WEAPON_COLOR_NUM];
	int weapon_color_is_new[MAX_WEAPON_COLOR_NUM];
	uint32_t n_weapon_color;
	FashionInfo fashion[MAX_FASHION_NUM];
	uint32_t n_fashion;
	uint32_t charm_level;
	uint32_t charm_total;

	//坐骑
	HorseInfo horse[MAX_HORSE_NUM];
	uint32_t n_horse;
	HorseCommonInfo horse_attr;

	//商城
	GoodsInfo shop_goods[MAX_SHOP_GOODS_NUM];

	//御气道
	YuqidaoMaiInfo yuqidao_mais[MAX_YUQIDAO_MAI_NUM];
	YuqidaoBreakInfo yuqidao_breaks[MAX_YUQIDAO_BREAK_NUM];

	uint32_t next_set_pktype_time;
	uint64_t qiecuo_target; //请求的切磋对象
	struct position qiecuo_pos;  //切磋的中点
	uint64_t qiecuo_start_time; //开始切磋的时间，没有在切磋的话这个值是0
	uint32_t qiecuo_out_range_fail_time; //如果当前时间超过这个时间，表示切磋的时候离开圈太久而失败

	struct pvp_raid_data pvp_raid_data;

	uint32_t m_collect_uuid;  //玩家当前正在采集的采集物的唯一ID
	uint32_t guild_id;
	uint32_t guild_office;
	uint32_t guild_donation;
	char guild_name[300];

	//八卦牌
	BaguapaiDressInfo baguapai_dress[MAX_BAGUAPAI_STYLE_NUM]; //八卦牌装备列表
	GuoYu guoyu;
	int cur_yaoshi;
	uint32_t change_special_num;
	ChengJie chengjie;
	ShangJin shangjin;
	ZhenYing zhenying;
	CommonAnswer common_answer;
	NpcAnswer award_answer;

	uint8_t camp_id;

	double m_angle;  //朝向，移动的时候设置

	//活动
	uint32_t active_reward[MAX_ACTIVE_REWARD_NUM]; //已领取的活跃度奖励
	DailyActivityInfo daily_activity[MAX_DAILY_ACTIVITY_NUM]; //日常活动
	ChivalryActivityInfo chivalry_activity[MAX_CHIVALRY_ACTIVITY_NUM]; //侠义活动

	uint8_t qiecuo_invite_switch; //切磋邀请开关
	uint8_t team_invite_switch; //组队邀请开关
	uint32_t out_stuck_time; //脱离卡死CD时间

	uint64_t next_update;

		//ai相关
	bool stop_ai; //停止AI
	uint8_t patrol_index; //巡逻路径
	uint8_t active_attack_range; //主动攻击范围
	uint8_t chase_range;  //追击范围

	//个人信息
	uint32_t personality_sex; //性别
	uint32_t personality_birthday; //生日（格式：20170410）
	char personality_location[MAX_PERSONALITY_LOCATION_LEN + 1]; //位置
	uint32_t personality_tags[MAX_PERSONALITY_TAG_NUM]; //标签
	char personality_text_intro[MAX_PERSONALITY_TEXT_INTRO_LEN + 1]; //文字签名
	char personality_voice_intro[MAX_PERSONALITY_VOICE_INTRO_LEN + 1]; //语音签名

		//自动补血
	uint64_t next_auto_add_hp_time; //下次血池补血时间或者脱战时间
	bool   on_fight_state;    //是否在战斗状态
	bool   open_auto_add_hp;
	uint32_t auto_add_hp_item_id; //自动补血的道具ID
	uint32_t auto_add_hp_percent; //自动补血的百分比，0-100
	uint32_t hp_pool_num;   //剩余的血池容量

	LiveSkill live_skill;
	//帮会技能属性
	double guild_skill_attr[PLAYER_ATTR_FIGHT_MAX]; //属性
	XunBaoData xunbao;

	EscortInfo escort_list[MAX_ESCORT_NUM]; //护送列表
	CashTruckData truck;

	//伙伴
	uint32_t partner_dictionary[MAX_PARTNER_TYPE];
	uint64_t partner_formation[MAX_PARTNER_FORMATION_NUM];
	uint64_t partner_battle[MAX_PARTNER_BATTLE_NUM];
	uint32_t partner_recruit_junior_time; //低级招募免费时间
	uint32_t partner_recruit_junior_count; //低级招募计数
	uint32_t partner_recruit_senior_time; //高级招募免费时间
	uint32_t partner_recruit_senior_count; //高级招募计数

	LeaveRaidPosition leaveraid; //离开副本
	uint32_t noviceraid_flag;	//新手副本是否完成的标记
	uint64_t Receive_type;		//即将开启，已经领取最大奖励的类型

	uint64_t world_chat_cd;

	uint32_t team_raid_id_wait_ready;    //在等待准备的副本ID
	bool is_team_raid_ready;  //是否ready

	uint32_t friend_num;
};


class player_struct: public unit_struct
{
public:
	typedef std::multimap<uint32_t, uint32_t> ItemPosMap;

	ItemPosMap item_pos_cache;
	TaskSet task_finish_set;
public:
	player_struct();
	virtual ~player_struct();
	player_data *data;   //放共享内存的数据，可以恢复模式下恢复
	void init_player();

	void send_enter_region_notify(int region_id);

	bool is_in_same_guild(player_struct *player);
	bool is_on_horse(void);
	bool is_on_truck(void);

	void refresh_player_redis_info(bool offline = false);
	void send_raid_earning_time_notify();
	void send_buff_info();
	void add_raid_reward_count(uint32_t raid_id);
	uint32_t get_raid_reward_count(uint32_t raid_id);
	
	void send_hp_pool_changed_notify();

	raid_struct *get_raid();
	UNIT_TYPE get_unit_type();		
	bool is_avaliable();
	uint32_t get_skill_id();
	uint64_t get_uuid();
	double *get_all_attr();
	double get_attr(uint32_t id);
	double *get_all_buff_fight_attr();
	double get_buff_fight_attr(uint32_t id);
	void clear_cur_skill();

	void add_attr(uint32_t id, double value);		
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
	void try_return_raid();
	void try_return_zhenying_raid();	
	void try_return_guild_wait_raid();	
	void try_return_guild_battle_raid();

	bool is_chengjie_target(uint64_t player_id);

	JobDefine get_job();
	uint32_t get_level();

	int transfer_to_new_scene_by_config(uint32_t transfer_id, EXTERN_DATA *extern_data);	
//	int transfer_to_new_scene(uint32_t scene_id, EXTERN_DATA *extern_data);
	int transfer_to_new_scene(uint32_t scene_id, double pos_x, double pos_y, double pos_z, double direct, EXTERN_DATA *extern_data);
	int transfer_to_new_scene_impl(scene_struct *new_scene, double pos_x, double pos_y, double pos_z, double direct, EXTERN_DATA *extern_data);
	int transfer_to_birth_position(EXTERN_DATA *extern_data); //将玩家传送回地图出生点
	int transfer_to_guild_scene(EXTERN_DATA *extern_data); //将玩家传送到帮会地图
	int transfer_out_guild_scene(EXTERN_DATA *extern_data); //将玩家拉出帮会地图

//	int get_pk_type();
	bool can_beattack();
	bool is_in_safe_region();
	int get_camp_id();
	void set_camp_id(int id);
	
	Team *get_team();
	
	void on_tick();
	void on_tick_10();
	void on_dead(unit_struct *killer);
	void on_hp_changed(int damage);
	void on_beattack(unit_struct *player, uint32_t skill_id, int32_t damage);
	void on_attack(unit_struct *target);
	void on_relive(uint32_t type);
	void on_repel(unit_struct *player);
	void interrupt();
	void refresh_oneday_job();  //每天刷新一次
	void refresh_shop_daily(void); //商城每日刷新

	bool is_online(void);
	bool is_in_raid();
	bool is_in_pvp_raid();	
//	bool is_player_in_sight(uint64_t other);

//	struct position *get_player_pos();

//sight相关
	int prepare_add_player_to_sight(player_struct *player);
	int prepare_add_monster_to_sight(monster_struct *monster);
	int prepare_add_truck_to_sight(cash_truck_struct *truck);
	int prepare_add_partner_to_sight(partner_struct *partner);	
	
	int get_sight_priority(player_struct *player);   //是不是要优先加入视野
//	bool is_player_in_sight(uint64_t player_id);

//	int add_player_to_sight(uint64_t player_id);
//	int del_player_from_sight(uint64_t player_id);

	bool on_player_leave_sight(uint64_t player_id);
	bool on_player_enter_sight(uint64_t player_id);
	bool on_monster_leave_sight(uint64_t uuid);
	bool on_monster_enter_sight(uint64_t uuid);
	bool on_partner_leave_sight(uint64_t player_id);
	bool on_partner_enter_sight(uint64_t player_id);

	void set_team_raid_id_wait_ready(uint32_t raid_id);
	void unset_team_raid_id_wait_ready();	

//both 系列函数
//player:
//	player, monster, truck, partner
//
//monster:
//	monster, truck, partner
//
//truck:
//	partner
//
//partner:
//	partner
	
	int add_player_to_sight_both(player_struct *player);
	int del_player_from_sight_both(player_struct *player);
	int add_monster_to_sight_both(monster_struct *monster);
	int del_monster_from_sight_both(monster_struct *monster);
	int add_cash_truck_to_sight_both(cash_truck_struct *truck);
	int del_cash_truck_from_sight_both(cash_truck_struct *truck);
	int add_partner_to_sight_both(partner_struct *partner);
	int del_partner_from_sight_both(partner_struct *partner);	
	
//	void broadcast_to_sight(uint16_t msg_id, void *msg_data, pack_func func, bool include_myself);

//	bool is_monster_in_sight(uint64_t uuid);
//	int add_monster_to_sight(uint64_t uuid);
//	int del_monster_from_sight(uint64_t uuid);
	void clear_player_sight();

	void process_offline(bool again = false, EXTERN_DATA *ext_data = NULL);
	void cache_to_dbserver(bool again = false, EXTERN_DATA *ext_data = NULL); //通过db_srv保存
	int pack_playerinfo_to_dbinfo(uint8_t *out_data);
	int unpack_dbinfo_to_playerinfo(uint8_t *packed_data, int len);
	void clear_temp_data();

	void broadcast_to_sight_and_team(uint16_t msg_id, void *msg_data, pack_func func, bool include_myself);
	
	void update_sight(area_struct *old_area, area_struct *new_area);
	int broadcast_player_create(scene_struct *scene);
	int broadcast_player_delete();
	void update_player_pos_and_sight();	
	void pack_sight_player_base_info(SightPlayerBaseInfo *info);
	void send_clear_sight();
	void send_clear_sight_monster();	
	void send_scene_transfer(float direct, float pos_x, float pos_y, float pos_z, uint32_t scene_id, int32_t result);

	//属性
	void calculate_attribute(bool isNty = false);
	void notify_attr(AttrMap& attr_list, bool broadcast = false, bool include_myself = true);
	void broadcast_one_attr_changed(uint32_t id, double value, bool send_team, bool include_myself);
	void notify_attr_changed(uint32_t num, uint32_t* id, double *value);
	void notify_one_attr_changed(uint32_t attr_id, double attr_val);
	
	void calcu_level_attr(double *attr); //等级属性
	void calcu_equip_attr(double *attr); //装备属性
	void calcu_yuqidao_attr(double *attr); //御气道属性
	void calcu_baguapai_attr(double *attr); //八卦牌属性
	void calcu_guild_skill_attr(double *attr); //帮会技能属性
	void calcu_partner_attr(double *attr); //伙伴属性
	void calcu_fashion_attr(double *attr); //伙伴属性

	uint32_t get_partner_fc(void); //获取伙伴模块的战力

	char *get_name();
	bool is_in_qiecuo();
	void set_qiecuo(uint32_t pos_x, uint32_t pos_z, uint64_t target);
	void finish_qiecuo();
	bool is_qiecuo_target(player_struct *target);

	void print_attribute(const char * stype, double *attr);

	//货币
	int get_comm_gold(void); //获取绑定+非绑定元宝
	int add_unbind_gold(uint32_t num, uint32_t statis_id, bool isNty = true); //增加非绑定元宝
	int add_bind_gold(uint32_t num, uint32_t statis_id, bool isNty = true); //增加绑定元宝
	int sub_unbind_gold(uint32_t num, uint32_t statis_id, bool isNty = true); //消耗非绑定元宝
	int sub_comm_gold(uint32_t num, uint32_t statis_id, bool isNty = true); //消耗通用元宝，先消耗绑定元宝
	int add_coin(uint32_t num, uint32_t statis_id, bool isNty = true); //增加银两
	int sub_coin(uint32_t num, uint32_t statis_id, bool isNty = true); //消耗银两
	uint32_t get_coin(void); //获取银两
	int add_zhenqi(uint32_t num, uint32_t statis_id, bool isNty = true); //增加真气
	int sub_zhenqi(uint32_t num, uint32_t statis_id, bool isNty = true); //消耗真气
	uint32_t get_zhenqi(void); //获取真气
	void add_guoyu_exp(uint32_t num);
	void add_chengjie_exp(uint32_t num);
	void add_chengjie_courage(uint32_t num);
	void add_shangjin_exp(uint32_t num);
	void refresh_yaoshi_oneday();
	void refresh_zhenying_task_oneday();
	void send_all_yaoshi_num();
	void add_zhenying_exp(uint32_t num);
	void send_zhenying_info();

	//背包
	void item_find_pos_by_cache(uint32_t id, std::vector<uint32_t>& pos_list); //通过id查找pos
	void add_item_pos_cache(uint32_t id, uint32_t pos);
	void del_item_pos_cache(uint32_t id, uint32_t pos);
	void create_item_cache(void);
	void fit_bag_grid_num(void);

	int check_can_transfer();

	uint32_t set_item_cd(ItemsConfigTable *config);	
	int check_item_cd(ItemsConfigTable *config);
	bool check_can_add_item(uint32_t id, uint32_t num, std::map<uint32_t, uint32_t> *out_add_list); //检查背包空间
	bool check_can_add_item_list(std::map<uint32_t, uint32_t>& item_list);
	int add_item(uint32_t id, uint32_t num, uint32_t statis_id, bool isNty = true); //增加道具
	bool add_item_list(std::map<uint32_t, uint32_t>& item_list, uint32_t statis_id, AddItemDealWay deal_way, bool isNty = true); //增加一堆道具
	int get_item_num_by_id(uint32_t id); //获取指定id道具数量
	int get_item_can_use_num(uint32_t id); //获取绑定+非绑定道具数量
	int del_item_grid(uint32_t pos, bool isNty = true);
	int del_item_by_pos(uint32_t pos, uint32_t num, uint32_t statis_id, bool isNty = true); //消耗指定格子道具
	int del_item_by_id(uint32_t id, uint32_t num, uint32_t statis_id, bool isNty = true); //消耗指定id道具
	int del_item(uint32_t id, uint32_t num, uint32_t statis_id, bool isNty = true); //消耗道具，先消耗绑定的
	int merge_item(uint32_t id); //将非绑定道具变成绑定道具
	int stack_item(uint32_t id); //调整多个格子堆叠数量
	void tidy_bag(void);
	int use_prop_effect(uint32_t id, std::map<uint32_t, uint32_t> *add_list);
	int use_prop(uint32_t pos, uint32_t use_all, std::map<uint32_t, uint32_t> *add_list);
	void update_bag_grid(uint32_t pos);
	bool is_item_expire(uint32_t pos); //判断道具是否到期
	void check_bag_expire(bool isNty = false);

	int move_baguapai_to_bag(BaguapaiCardInfo &card);

	//掉落
	bool give_drop_item(uint32_t drop_id, uint32_t statis_id, AddItemDealWay deal_way, bool isNty = true); //发放掉落奖励

	//经验
	int add_exp(uint32_t val, uint32_t statis_id, bool isNty = true);
	int deal_level_up(uint32_t level_old, uint32_t level_new);
	int get_total_exp(void);
	int sub_exp(uint32_t val, uint32_t statis_id, bool isNty = true); //减少经验

	//头像
	int add_head_icon(uint32_t icon_id);
	int init_head_icon(void);
	HeadIconInfo *get_head_icon(uint32_t icon_id);
	void check_head_condition(uint32_t condition_id, uint32_t condition_val);

	//任务
	bool check_task_accept_condition(uint32_t type, uint32_t target, uint32_t val);
	bool check_task_accept_condition_by_id(uint32_t id);
	bool task_is_finish(uint32_t task_id);
	int task_is_acceptable(uint32_t task_id);
	TaskInfo *get_task_info(uint32_t task_id);
	int add_task(uint32_t task_id, uint32_t status, bool isNty = false);
	void task_update_notify(TaskInfo *info);
	void get_task_event_item(uint32_t task_id, uint32_t event_class, std::map<uint32_t, uint32_t> &item_list);
	int touch_task_event(uint32_t task_id, uint32_t event_class);
	int execute_task_event(uint32_t event_id, uint32_t event_class);
	int add_finish_task(uint32_t task_id);
	int del_finish_task(uint32_t task_id);
	int submit_task(uint32_t task_id);
	void get_task_reward_item(uint32_t task_id, std::map<uint32_t, uint32_t> &item_list);
	int give_task_reward(uint32_t task_id);
	int give_task_reward_by_reward_id(uint32_t reward_id, uint32_t statis_id);
	int touch_task_drop(uint32_t scene_id, uint32_t monster_id);
	void load_task_end(void);

	void init_task_progress(TaskInfo *info);
	void add_task_progress(uint32_t type, uint32_t target, uint32_t num, uint32_t task_id = 0, uint32_t cond_id = 0, uint64_t teammate_id = 0);
	bool task_is_achieved(TaskInfo *info);
	int set_task_fail(TaskInfo *info);
	static bool task_condition_can_fail(uint32_t task_id);

	void logout_check_task_time(void);
	void logout_check_award_question(void);
	void login_check_task_time(void);
	void check_task_time(void);
	uint32_t get_task_expire_time(TaskInfo *info);

	int accept_task(uint32_t task_id, bool check_condition = true);
	void clear_team_task(void); //清除整个队伍共享的任务
	void task_remove_notify(uint32_t task_id);
	void task_finish_add_notify(uint32_t task_id);
	void task_finish_del_notify(uint32_t task_id);
	void leave_team(player_struct *leader);
	void hand_out_team_leader(player_struct *leader); //把组队队长数据移交给新队长

	int get_task_chapter_info(uint32_t &id, uint32_t &state);
	void update_task_chapter_info(void);

	void do_taunt_action();
	void update_region_id();

	void on_region_changed(uint16_t old_region_id, uint16_t new_region_id);

	//吟唱
	void sing_notify(uint32_t msg_id, uint32_t type, uint32_t time, bool broadcast, bool include_myself);
	int begin_sing(uint32_t type, uint32_t time, uint32_t obj_id, bool broadcast, bool include_myself);
	int interrupt_sing(void);
	int end_sing(void);

	//系统提示
	void send_system_notice(uint32_t id, std::vector<char*> &args);
	static void send_rock_notice(player_struct &player, uint32_t notify_id);

	int conserve_out_raid_pos_and_scene(raid_struct *raid); //进入副本前保存离开副本后所到的场景id以及位置
	int set_enter_raid_pos_and_scene(raid_struct *raid, double pos_x, double pos_z);	
	int set_out_raid_pos_and_clear_scene();
	int set_out_raid_pos();
	

	//时装
	int add_fashion(uint32_t id , uint32_t color, time_t expire);
	void set_fashion_old(uint32_t id);
	void set_fashion_color(uint32_t id, uint32_t color, bool down);
	void unlock_color(uint32_t color);
	void unlock_weapon_color(uint32_t color);
	int get_fashion(uint32_t id);
	int get_color(uint32_t color);
	int get_weapon_color(uint32_t color);
	void check_fashion_expire();
	void open_new_fashion(uint32_t level_old, uint32_t level_new);


	//坐骑
	int add_horse(uint32_t id, time_t expire);
	void notify_add_horse(int i);
	int get_horse(uint32_t id);
	int get_on_horse_id();
	void unpack_horse(_PlayerDBInfo *db_info);
	void calc_horse_attr(double *attr);
	void calc_horse_attr();
	void check_horse_expire();
	void check_guoyu_expire();
	bool go_down_cash_truck();

	//装备
	EquipInfo *get_equip(uint32_t type);
	int add_equip(uint32_t type, uint32_t statis_id);
	int add_equip_exp(uint32_t type, uint32_t val);
	bool equip_is_max_star(uint32_t type);
	uint32_t get_equip_max_star_need_exp(uint32_t type); //升到当前最大星需要的经验
	void update_weapon_skin(bool isNty); //更新武器外形

		//pvp副本
	int change_pvp_raid_score(int type, int value);
	int add_today_pvp_win_num(int type);
	int send_pvp_raid_score_changed(int type);
	uint32_t pvp_raid_cancel_time;  //pvp副本匹配CD

		//清除类型3(变身)buff
	bool is_in_buff3();
	void clear_type3_buff();
	void clear_god_buff();
	void clear_one_buff(uint32_t id);	
	uint32_t add_murder_num(uint32_t num); //添加杀戮值, 返回修改后的杀戮值
	uint32_t sub_murder_num(uint32_t num);	//减少杀戮值, 返回修改后的杀戮值

	void add_pet(monster_struct *pet);
	void del_pet(monster_struct *pet);

	//邮件
	int send_mail(uint32_t type, char *title = NULL, char *sender_name = NULL, char *content = NULL, std::vector<char *> *args = NULL, std::map<uint32_t, uint32_t> *attachs = NULL, uint32_t statis_id = 0); //发邮件，注意一封邮件最多带6种附件
	int send_mail_by_id(uint32_t type, std::vector<char *> *args = NULL, std::map<uint32_t, uint32_t> *attachs = NULL, uint32_t statis_id = 0);

	//御气道
	int init_yuqidao_mai(uint32_t break_id, bool isNty); //通过冲脉激活新经脉
	YuqidaoMaiInfo *get_yuqidao_mai(uint32_t mai_id);
	int init_yuqidao_break(uint32_t break_id); //经脉升满激活冲脉
	YuqidaoBreakInfo *get_yuqidao_break(uint32_t break_id);

	//八卦牌
	BaguapaiDressInfo *get_baguapai_dress(uint32_t style_id);
	BaguapaiCardInfo *get_baguapai_card(uint32_t style_id, uint32_t part_id);
	int generate_baguapai_main_attr(uint32_t card_id, double &attr_val);
	int generate_baguapai_minor_attr(uint32_t card_id, AttrInfo *attrs);

	//活动
	int add_activeness(uint32_t num, uint32_t statis_id, bool isNty = true);
	uint32_t get_activeness(void);
	bool activity_is_unlock(uint32_t act_id);
	int check_activity_progress(uint32_t matter, uint32_t value);
	int activity_finish_check_chivalry(uint32_t chivalry_id);
	int add_chivalry(uint32_t num, uint32_t statis_id, bool isNty = true);
	int sub_chivalry(uint32_t num, uint32_t statis_id, bool isNty = true);
	uint32_t get_chivalry(void);
	void update_daily_activity_item(DailyActivityInfo *info);
	void update_chivalry_activity_item(ChivalryActivityInfo *info);
	void refresh_activity_daily(void);
	void notify_activity_info(EXTERN_DATA *extern_data);

	void add_wanyaoka(uint32_t *id, uint32_t n_id);
	void clear_award_question();
	

	//帮会
	void add_guild_resource(uint32_t type, uint32_t num);
	void sub_guild_building_time(uint32_t time);
	void disband_guild(uint32_t guild_id);

	//护送
	int start_escort(uint32_t escort_id);
	int stop_escort(uint32_t escort_id, bool success);
	void stop_all_escort(void);
	EscortInfo *get_escort_info(uint32_t escort_id);
	int clear_escort_by_id(uint32_t escort_id);
	int clear_escort_by_index(uint32_t idx);
	void clear_all_escort(void);
	void check_escort(void);

	//伙伴
	partner_struct *get_battle_partner();
	int add_partner(uint32_t partner_id, uint64_t *uuid = NULL);
	int add_partner_dictionary(uint32_t partner_id);
	int remove_partner(uint64_t partner_uuid);
	void clear_all_partners(void);
	partner_struct *get_partner_by_uuid(uint64_t partner_uuid);
	bool partner_is_in_formation(uint64_t partner_uuid);
	bool partner_is_in_battle(uint64_t partner_uuid);
	void load_partner_end(void);
	bool is_partner_battle(void); //伙伴是否参战
	bool is_partner_precedence(void); //主战伙伴是否优先出战
	int add_partner_to_scene(uint64_t partner_uuid); //把伙伴加入到玩家所在的场景
	int del_partner_from_scene(uint64_t partner_uuid);
	int del_partner_from_scene(partner_struct *partner);
	void del_battle_partner_from_scene();  //临时把所有出战伙伴移出场景(副本结束的时候用)
	void take_partner_into_scene(void); //角色进入场景后，把伙伴加入场景
	void adjust_battle_partner(void); //阵型变化，调整出战的伙伴
	uint64_t get_next_can_battle_partner(void); //获取能出战的伙伴
	uint64_t get_fighting_partner(void);
	void on_leave_scene(scene_struct *old_scene);
	int add_partner_anger(uint32_t num, bool isNty = true);
	int reset_partner_anger(bool isNty = true);
	int add_partner_exp(uint32_t num, uint32_t statis_id, bool isNty = true);
	void notify_fighting_partner(void);
	void check_partner_relive(void);

	uint64_t last_change_area_time;
	sight_space_struct *sight_space;
//	raid_struct *m_raid;
	Team *m_team;
	std::set<uint64_t> m_inviter; 
	MySkill m_skill;
	std::set<monster_struct *> m_pet_list;
	std::vector<uint64_t> bagua_buffs;
	std::deque<uint64_t> m_hitMe;
	std::set<uint64_t> m_meHit;
	uint32_t guild_battle_wait_award_time;
	std::map<uint32_t, uint32_t> xun_map_id;  //second: 藏宝图对应的出生点, first:藏宝图ID(物品表)
	PartnerMap m_partners;
	uint32_t chengjie_kill; //悬赏目标被杀

		//ai巡逻配置
	struct RobotPatrolTable *ai_patrol_config;
private:
	void use_hp_pool_add_hp();
	void enter_fight_state();
	void leave_fight_state();	
	void do_auto_add_hp();
	void pack_answer_db(_PlayerDBInfo &db_info);
	void unpack_answer_db(_PlayerDBInfo *db_info);
	int add_pvp_raid_score(int type, int value);
	int sub_pvp_raid_score(int type, int value);
	void on_relive_in_raid(raid_struct *raid, uint32_t type);
	void on_kill_player(player_struct *dead);	
	void send_raid_hit_statis(raid_struct *raid);	
	void try_out_raid();
	void check_qiecuo_range();	
	void del_sight_player_in_area(int n_del, area_struct **del_area, int *delete_player_id_index, uint64_t *delete_player_id);
	void del_sight_monster_in_area(int n_del, area_struct **del_area, int *delete_monster_uuid_index, uint64_t *delete_monster_uuid);
	void del_sight_partner_in_area(int n_del, area_struct **del_area, int *delete_partner_uuid_index, uint64_t *delete_partner_uuid);	
	void del_sight_truck_in_area(int n_del, area_struct **del_area, int *delete_truck_uuid_index, uint64_t *delete_truck_uuid);
	void add_area_player_to_sight(area_struct *area, int *add_player_id_index, SightPlayerBaseInfo *add_player);
	void add_area_monster_to_sight(area_struct *area, int *add_monster_id_index, SightMonsterInfo *add_monster);
	void add_area_collect_to_sight(area_struct *area, int *add_collect_id_index, SightCollectInfo *add_collect);
	void add_area_truck_to_sight(area_struct *area, int *add_truck_id_index, SightCashTruckInfo *add_truck);
	void add_area_partner_to_sight(area_struct *area, int *add_partner_id_index, SightPartnerInfo *add_partner);	
//	void add_area_player_to_sight(area_struct *area, int *add_player_id_index, uint64_t *add_player_id);	
	uint32_t get_first_skill_id();
	uint64_t reset_pvp_raid_level(uint16_t score, uint8_t *level, uint8_t *star);	
	void update_rtt(uint32_t stamp);
	int64_t srtt;       //网络延迟 * 8
};

void init_sight_unit_info_point();
bool notice_use_art(uint32_t statis_id);
int check_qiecuo_finished(player_struct *p1, player_struct *p2);
#endif
